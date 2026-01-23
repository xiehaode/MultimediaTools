#include "pch.h"
#include "../OpenCVTools/OpenCVFFMpegTools.h"


TEST(TestCaseName, TestName) {
  EXPECT_EQ(1, 1);
  EXPECT_TRUE(true);

  void* worker = AvWorker_Create();
  ASSERT_NE(worker, nullptr);

  const bool ok = AvWorker_GetVideoFirstFrame(worker, "1.mp4", "1.bmp", false);
  
  EXPECT_TRUE(ok);
  const bool ok2 = AvWorker_SpliceAV(worker, "1.mp4", "2.mp4", "3.mp4", false);
  EXPECT_TRUE(ok2);
  AvWorker_Destroy(worker);
}
