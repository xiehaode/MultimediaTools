#include "mdevice.h"
#include <windows.h>
#include <dshow.h>
#include <QDebug>  // 增加Qt日志，方便调试
#include <cstdio>

// 确保HDevice结构体在mdevice.h中正确定义
// struct HDevice {
//     char name[256];  // 足够大的缓冲区存储设备名
// };

static std::vector<HDevice> getDevicesList(REFGUID category) {
    std::vector<HDevice> ret;
    HRESULT hr = S_OK;

    // 1. 初始化COM环境，禁止覆盖返回值，增加错误校验
    /*
    hr = CoInitializeEx(NULL, COINIT_MULTITHREADED);
    if (FAILED(hr)) {
        qCritical() << "[COM初始化失败] 错误码：" << hr
                    << "，原因：线程已初始化/权限不足/COM环境异常";
        return ret;
    }
    */
    ICreateDevEnum *pDevEnum = NULL;
    // 2. 创建设备枚举器，增加错误日志
    hr = CoCreateInstance(CLSID_SystemDeviceEnum, NULL,
                          CLSCTX_INPROC_SERVER, IID_PPV_ARGS(&pDevEnum));
    if (FAILED(hr) || !pDevEnum) {
        qCritical() << "[创建设备枚举器失败] 错误码：" << hr;
        CoUninitialize();  // 释放COM，与CoInitializeEx成对
        return ret;
    }

    IEnumMoniker *pEnum = NULL;
    // 3. 创建类枚举器，区分S_FALSE（无设备）和E_XXX（调用失败）
    hr = pDevEnum->CreateClassEnumerator(category, &pEnum, 0);
    if (hr == S_FALSE) {
        qWarning() << "[device enum]no find enumerator category:" << &category;
        pDevEnum->Release();  // 释放已创建的枚举器
        CoUninitialize();     // 释放COM
        return ret;
    }
    if (FAILED(hr) || !pEnum) {
        qCritical() << "[创建类枚举器失败] 错误码：" << hr;
        pDevEnum->Release();
        CoUninitialize();
        return ret;
    }

    // 4. 遍历设备别名器
    IMoniker *pMoniker = NULL;
    ULONG fetched = 0;  // 必须传入ULONG*，原代码传NULL存在潜在bug
    while (pEnum->Next(1, &pMoniker, &fetched) == S_OK && pMoniker && fetched > 0) {
        IPropertyBag *pPropBag = NULL;
        hr = pMoniker->BindToStorage(0, 0, IID_PPV_ARGS(&pPropBag));
        if (FAILED(hr) || !pPropBag) {
            qWarning() << "[绑定属性袋失败] 跳过该设备，错误码：" << hr;
            pMoniker->Release();
            continue;
        }

        VARIANT var;
        VariantInit(&var);
        HDevice dev;
        memset(dev.name, 0, sizeof(dev.name));  // 初始化缓冲区，避免脏数据

        // 5. 多属性兼容读取：Description → FriendlyName → DevicePath，解决部分设备无名称问题
        hr = pPropBag->Read(L"Description", &var, 0);
        if (FAILED(hr)) {
            hr = pPropBag->Read(L"FriendlyName", &var, 0);
        }
        if (FAILED(hr)) {
            hr = pPropBag->Read(L"DevicePath", &var, 0);
            qWarning() << "设备无标准名称，使用DevicePath标识";
        }

        // 6. 成功读取属性，转换为char并加入结果集
        if (SUCCEEDED(hr)) {
            // %S 匹配宽字符bstrVal，确保中文/特殊字符正常显示
            snprintf(dev.name, sizeof(dev.name) - 1, "%S", var.bstrVal);
            ret.push_back(dev);
            qInfo() << "[找到设备] " << dev.name;  // 打印找到的设备，方便调试
            VariantClear(&var);  // 释放VARIANT，避免内存泄漏
        } else {
            qWarning() << "[读取设备属性失败] 跳过该设备，错误码：" << hr;
        }

        // 7. 释放当前设备的COM资源，逐个释放，避免泄漏
        pPropBag->Release();
        pMoniker->Release();
    }

    // 8. 释放所有全局COM资源，按创建逆序释放
    pEnum->Release();
    pDevEnum->Release();
    //CoUninitialize();  // 与CoInitializeEx严格成对

    qInfo() << "find device num" << ret.size() ;
    return ret;
}

// 视频输入设备枚举（摄像头/采集卡）
std::vector<HDevice> getVideoDevices() {
    qInfo() << "开始枚举视频输入设备...";
    return getDevicesList(CLSID_VideoInputDeviceCategory);
}

// 音频输入设备枚举（麦克风）
std::vector<HDevice> getAudioDevices() {
    qInfo() << "开始枚举音频输入设备...";
    return getDevicesList(CLSID_AudioInputDeviceCategory);
}
