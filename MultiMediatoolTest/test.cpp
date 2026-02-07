#include "pch.h"
#include "../OpenCVTools/OpenCVFFMpegTools.h"
#include "../formatChange/formatChange.h"
#include <string>

#include <Windows.h>

#if defined(__has_include)
#if __has_include(<Python.h>)
#include <Python.h>
#define HAS_PYTHON_H 1
#else
#define HAS_PYTHON_H 0
#endif
#else
#define HAS_PYTHON_H 0
#endif

#include <cstdio>
#include <stdio.h>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <queue>
#include <string>
#include <memory>

extern "C" int printf(const char* format, ...);

#ifndef HAS_PYTHON_H
#define HAS_PYTHON_H 0
#endif


struct PyTask {
	enum class Action {
		GetSupportedConversions,
		Convert
	};

	Action action;
	std::string input;
	std::string output;
	bool done = false;
	bool ok = false;
	std::string result;
	std::string error;
	std::mutex mtx;
	std::condition_variable cv;
};

class PythonWorker {
public:
	PythonWorker(const char* module_dir)
		: module_dir_(module_dir) {}

	bool Start() {
#if !HAS_PYTHON_H
		(void)module_dir_;
		return false;
#else
		worker_ = std::thread([this]() { this->Run(); });
		return true;
#endif
	}

	void Stop() {
		{
			std::lock_guard<std::mutex> lock(queue_mtx_);
			stopping_ = true;
		}
		queue_cv_.notify_all();
		if (worker_.joinable()) {
			worker_.join();
		}
	}

	void Enqueue(const std::shared_ptr<PyTask>& task) {
		{
			std::lock_guard<std::mutex> lock(queue_mtx_);
			queue_.push(task);
		}
		queue_cv_.notify_one();
	}

private:
	void Run() {
#if HAS_PYTHON_H
		Py_Initialize();
		PyEval_InitThreads();

		PyGILState_STATE gstate = PyGILState_Ensure();
		PyObject* sys_path = PySys_GetObject("path");
		if (sys_path) {
			PyObject* p = PyUnicode_FromString(module_dir_.c_str());
			if (p) {
				PyList_Append(sys_path, p);
				Py_DECREF(p);
			}
		}

		PyObject* module = PyImport_ImportModule("universal_converter");
		if (module) {
			PyObject* cls = PyObject_GetAttrString(module, "UniversalConverter");
			if (cls) {
				converter_ = PyObject_CallObject(cls, NULL);
				Py_DECREF(cls);
			}
			Py_DECREF(module);
		}
		if (!converter_) {
			PyErr_Print();
		}
		PyGILState_Release(gstate);

		while (true) {
			std::shared_ptr<PyTask> task;
			{
				std::unique_lock<std::mutex> lock(queue_mtx_);
				queue_cv_.wait(lock, [&]() { return stopping_ || !queue_.empty(); });
				if (stopping_ && queue_.empty()) {
					break;
				}
				task = queue_.front();
				queue_.pop();
			}

			HandleTask(task);
		}

		PyGILState_STATE gstate_end = PyGILState_Ensure();
		Py_XDECREF(converter_);
		converter_ = NULL;
		PyGILState_Release(gstate_end);
		Py_Finalize();
#endif
	}

	void HandleTask(const std::shared_ptr<PyTask>& task) {
		if (!task) {
			return;
		}

		PyGILState_STATE gstate = PyGILState_Ensure();
		if (!converter_) {
			task->ok = false;
			task->error = "converter not initialized";
			PyGILState_Release(gstate);
			NotifyTask(task);
			return;
		}

		switch (task->action) {
		case PyTask::Action::GetSupportedConversions:
		{
			PyObject* conversions = PyObject_CallMethod(converter_, "get_supported_conversions", NULL);
			if (!conversions) {
				PyErr_Print();
				task->ok = false;
				task->error = "get_supported_conversions failed";
				break;
			}
			PyObject* json_mod = PyImport_ImportModule("json");
			PyObject* dumps = json_mod ? PyObject_GetAttrString(json_mod, "dumps") : NULL;
			if (!dumps) {
				Py_XDECREF(json_mod);
				Py_DECREF(conversions);
				task->ok = false;
				task->error = "json.dumps not available";
				break;
			}
			PyObject* args = PyTuple_Pack(1, conversions);
			PyObject* kwargs = PyDict_New();
			PyDict_SetItemString(kwargs, "ensure_ascii", Py_False);
			PyObject* json_str = PyObject_Call(dumps, args, kwargs);
			if (json_str) {
				const char* s = PyUnicode_AsUTF8(json_str);
				task->result = s ? s : "";
				task->ok = true;
				Py_DECREF(json_str);
			} else {
				PyErr_Print();
				task->ok = false;
				task->error = "json.dumps failed";
			}
			Py_DECREF(kwargs);
			Py_DECREF(args);
			Py_DECREF(dumps);
			Py_XDECREF(json_mod);
			Py_DECREF(conversions);
			break;
		}
		case PyTask::Action::Convert:
		{
			PyObject* result = PyObject_CallMethod(converter_, "convert", "ss", task->input.c_str(), task->output.c_str());
			if (!result) {
				PyErr_Print();
				task->ok = false;
				task->error = "convert failed";
			} else {
				task->ok = PyObject_IsTrue(result) ? true : false;
				Py_DECREF(result);
			}
			break;
		}
		default:
			task->ok = false;
			task->error = "unknown action";
			break;
		}

		PyGILState_Release(gstate);
		NotifyTask(task);
	}

	void NotifyTask(const std::shared_ptr<PyTask>& task) {
		std::lock_guard<std::mutex> lock(task->mtx);
		task->done = true;
		task->cv.notify_one();
	}

private:
	std::string module_dir_;
	std::thread worker_;
	std::mutex queue_mtx_;
	std::condition_variable queue_cv_;
	std::queue<std::shared_ptr<PyTask>> queue_;
	bool stopping_ = false;
	PyObject* converter_ = NULL;
};

static void TestUniversalConverterPython()
{
#if !HAS_PYTHON_H
	printf("未找到 Python.h，请设置 PYTHON_HOME 并重新生成。\n");
	return;
#else
	PythonWorker worker("D:/vsPro/MultiMediaTool/WordToPdf");
	if (!worker.Start()) {
		printf("PythonWorker 启动失败\n");
		return;
	}

	auto task = std::make_shared<PyTask>();
	task->action = PyTask::Action::GetSupportedConversions;
	worker.Enqueue(task);

	{
		std::unique_lock<std::mutex> lock(task->mtx);
		task->cv.wait(lock, [&]() { return task->done; });
	}

	if (task->ok) {
		printf("支持的转换: %s\n", task->result.c_str());
	} else {
		printf("获取失败: %s\n", task->error.c_str());
	}

	worker.Stop();
#endif
}

int main() {
	TestUniversalConverterPython();
	/*
	// 1. ��������/���·��
	// ʹ�þ���·�����������ʱ����Ŀ¼�仯�����Ҳ����ļ�
	const char* inputVideo = "D:/vsPro/MultiMediaTool/bin/1.mp4";
	const char* outputVideo = "D:/vsPro/MultiMediaTool/bin/output_addTextWatermark.mp4";


	// 2. ����videoTrans�����C�ӿڣ�VideoTrans_Create��
	void* pTrans = VideoTrans_Create();
	if (!pTrans) {
		printf("����VideoTrans���ʧ�ܣ�\n");
		return -1;
	}

	// 3. ��ʼ����C�ӿڣ�VideoTrans_Initialize��
	int initRet = VideoTrans_Initialize(pTrans, inputVideo, outputVideo);
	if (initRet != 0) {
		printf("��ʼ��ʧ�ܣ������룺%d\n", initRet);
		VideoTrans_Destroy(pTrans);
		return initRet;
	}

	// 4. ��ӡ��Ƶ���ԣ�C�ӿڻ�ȡ��/��/֡��/ʱ����
	int width = VideoTrans_GetWidth(pTrans);
	int height = VideoTrans_GetHeight(pTrans);
	int fps = VideoTrans_GetFPS(pTrans);
	int64_t duration = VideoTrans_GetDuration(pTrans);
	printf("��Ƶ���ԣ�%dx%d | %dfps | ʱ����%llds\n",
		width, height, fps, duration / 1000);

	// 5. ִ����Ƶ������C�ӿڣ�VideoTrans_Process��������Ч����intֵ��
	// ��Ч���ͣ�applyMosaic�������ˣ���ֱ�Ӵ�ö�ٶ�Ӧ��intֵ
	int processRet = VideoTrans_Process(pTrans, addTextWatermark);
	if (processRet != 0) {
		printf("��Ƶ����ʧ�ܣ������룺%d\n", processRet);
		VideoTrans_Destroy(pTrans);
		return processRet;
	}

	// 6. ���ý���������ѡ�����´���ͬһ·����Ƶʱʹ�ã�
	// int resetRet = VideoTrans_Reset(pTrans);
	// if (resetRet != 0) { printf("����ʧ�ܣ�%d\n", resetRet); }

	// 7. �ֶ�������Դ����ѡ��Destroy���Զ����ã�
	// VideoTrans_Cleanup(pTrans);

	// 8. ���پ�����ͷ�������Դ��������ã�
	VideoTrans_Destroy(pTrans);
	pTrans = nullptr;

	printf("��Ƶ������ɣ����·����%s\n", outputVideo);
	*/
	/*
	printf("===== ����ת��װ���� =====\n");
	// ����AVProcessorʵ��
	void* processor = AVProcessor_Create();
	if (!processor) {
		printf("����AVProcessorʵ��ʧ�ܣ�\n");
		return -1;
	}

	// ����ת��װ������MP4�����FLV
	const char* input_path = "D:/vsPro/MultiMediaTool/bin/1.mp4";
	const char* output_path = "test_output.flv";
	int ret = AVProcessor_Remux(processor, input_path, output_path);
	if (ret == 0) {
		printf("ת��װ�ɹ���%s �� %s\n", input_path, output_path);
	}
	else {
		printf("ת��װʧ�ܣ������룺%d\n", ret);
	}

	// ======================================
	// 2. ����ʾ��������ת�빦�ܣ����ò�����
	// ======================================
	printf("\n===== ����ת�빦�ܿ�� =====\n");
	AVConfig config = { 0 };
	// ����ת������
	config.width = 1280;          // �������
	config.height = 720;         // ����߶�
	//config.bitrate = 2000000;    // 2Mbps������
	//config.fps = 30;             // 30֡/��
	//strcpy(config.codec_name, "h264"); // H.264������

	// ����ת�룺����MP4�������MP4���޸ķֱ���/���룩
	const char* transcode_input = "2.mp4";
	const char* transcode_output = "test_transcode.mp4";
	ret = AVProcessor_Transcode(processor, transcode_input, transcode_output, &config);
	if (ret == 0) {
		printf("ת���ܵ��óɹ����貹�����������߼���\n");
	}
	else {
		printf("ת�����ʧ�ܣ������룺%d\n", ret);
	}

	// ======================================
	// 3. ����MP4תGIF����ܣ�
	// ======================================
	printf("\n===== ����MP4תGIF���ܿ�� =====\n");
	AVConfig gif_config = { 0 };
	gif_config.width = 640;
	gif_config.height = 360;
	//gif_config.fps = 10; // GIF֡��

	const char* gif_input = "D:/vsPro/MultiMediaTool/bin/1.mp4";
	const char* gif_output = "test_output.gif";
	ret = AVProcessor_Mp4ToGif(processor, gif_input, gif_output, &gif_config);
	if (ret == 0) {
		printf("MP4תGIF��ܵ��óɹ����貹�������߼���\n");
	}
	else {
		printf("MP4תGIF����ʧ�ܣ������룺%d\n", ret);
	}

	// ======================================
	// 4. ����ͼƬ����תMP4����ܣ�
	// ======================================
	printf("\n===== ����ͼƬ����תMP4���ܿ�� =====\n");
	AVConfig img_config = { 0 };
	img_config.width = 1920;
	img_config.height = 1080;
	//img_config.fps = 25;
	//strcpy(img_config.codec_name, "h264");

	const char* img_output = "D:/vsPro/MultiMediaTool/bin/1.mp4";
	ret = AVProcessor_ImgSeqToMp4(processor, img_output, &img_config);
	if (ret == 0) {
		printf("ͼƬ����תMP4��ܵ��óɹ����貹�������߼���\n");
	}
	else {
		printf("ͼƬ����תMP4����ʧ�ܣ������룺%d\n", ret);
	}

	// ======================================
	// 5. �ͷ���Դ
	// ======================================
	AVProcessor_Destroy(processor);
	printf("\n���в�����ɣ���Դ���ͷţ�\n");
	return 0;
	*/
}