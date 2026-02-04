#pragma once
#include "COpenCVTools.h"
#include "CvTranslator.h"
#include "FFmpegDecoder.h"
#include "FFmpegEncoder.h"
#include "COpenCVTools.h"
class videoTrans
{
public:
	videoTrans();
	int trans(func fun);
private:
	FFmpegDecoder* decoder;
	FFmpegEncoder* encoder;
	

};

