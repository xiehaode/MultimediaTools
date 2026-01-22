#include "pch.h"
#include "../OpenCVTools/OpenCVFFMpegTools.h"


TEST(TestCaseName, TestName) {
  EXPECT_EQ(1, 1);
  EXPECT_TRUE(true);

  void* worker = AvWorker_Create();
  ASSERT_NE(worker, nullptr);

  const bool ok = AvWorker_GetVideoFirstFrame(worker, "1.mp4", "1.bmp", false);
  EXPECT_TRUE(ok);

  AvWorker_Destroy(worker);
}
