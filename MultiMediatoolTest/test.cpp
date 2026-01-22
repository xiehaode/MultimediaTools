#include "pch.h"
#include "../OpenCVTools/AvWorker.h"


TEST(TestCaseName, TestName) {
  EXPECT_EQ(1, 1);
  EXPECT_TRUE(true);
  AvWorker worker;
  worker.GetVideoFirstFrame("1.mp4", "1.bmp");

}