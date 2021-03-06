cmake_minimum_required(VERSION 2.4.6)
include($ENV{ROS_ROOT}/core/rosbuild/rosbuild.cmake)

# Set the build type.  Options are:
#  Coverage       : w/ debug symbols, w/o optimization, w/ code-coverage
#  Debug          : w/ debug symbols, w/o optimization
#  Release        : w/o debug symbols, w/ optimization
#  RelWithDebInfo : w/ debug symbols, w/ optimization
#  MinSizeRel     : w/o debug symbols, w/ optimization, stripped binaries
#set(ROS_BUILD_TYPE RelWithDebInfo)

rosbuild_init()

#set the default path for built executables to the "bin" directory
set(EXECUTABLE_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/bin)
#set the default path for built libraries to the "lib" directory
set(LIBRARY_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/lib)

#uncomment if you have defined messages
#rosbuild_genmsg()
#uncomment if you have defined services
#rosbuild_gensrv()

INCLUDE_DIRECTORIES(${PROJECT_SOURCE_DIR}/ros/include
		${PROJECT_SOURCE_DIR}/common/include)

#common commands for building c++ executables and libraries
#rosbuild_add_library(${PROJECT_NAME} src/example.cpp)
#target_link_libraries(${PROJECT_NAME} another_library)
find_package(Boost 1.42.0)
rosbuild_add_boost_directories()
#rosbuild_link_boost(${PROJECT_NAME} thread)
#rosbuild_add_executable(example examples/example.cpp)
#target_link_libraries(example ${PROJECT_NAME})
rosbuild_add_roslaunch_check(ros/launch)

rosbuild_add_executable(read_label ros/src/read_label.cpp
                                 common/src/MatchTemplate.cpp
 									common/src/FeatureRepresentation.cpp
 									common/src/TextTagDetection.cpp
 									common/src/LBPandHistogram.cpp)

rosbuild_add_executable(match_template_node common/src/MatchTemplateNode.cpp common/src/MatchTemplate.cpp)
rosbuild_link_boost(match_template_node filesystem system)

rosbuild_add_executable(extract_features_and_reconition common/src/ExtractFeaturesAndReconition.cpp
                                   common/src/MatchTemplate.cpp
                                   common/src/LBPandHistogram.cpp)
                                   
rosbuild_add_executable(feature_representation_node common/src/FeatureRepresentationNode.cpp 
											common/src/LBPandHistogram.cpp
											common/src/FeatureRepresentation.cpp)

rosbuild_add_executable(text_tag_detection_node common/src/TextTagDetectionNode.cpp
											common/src/TextTagDetection.cpp)

#rosbuild_add_executable(text_tag_recogntion_test common/src/text_tag_recogntion_test.cpp 
#											common/src/LBPandHistogram.cpp
#											common/src/FeatureRepresentation.cpp
#											common/src/MatchTemplate.cpp)	
											
#rosbuild_add_executable(CutTagFromBackground2 common/src/CutTagFromBackground2.cpp)
#rosbuild_add_executable(detect_dashes_by_contour common/src/detect_dashes_by_contour.cpp)

#rosbuild_add_executable(resize common/src/resize.cpp)
#rosbuild_link_boost(resize filesystem system)

#rosbuild_add_executable(create_negatives_from_video common/src/CreateNegativesFromVideo.cpp)

#rosbuild_add_executable(puttext common/src/puttext.cpp)

#rosbuild_add_executable(combine_positives_and_negatives common/src/CombinePositivesAndNegatives.cpp)
#rosbuild_link_boost(resize filesystem system)

#rosbuild_add_executable(create_positive_images common/src/CreatePositiveImages.cpp)
#rosbuild_link_boost(create_positive_images filesystem system)

#rosbuild_add_executable(capture_image common/src/CaptureImage.cpp)
#rosbuild_link_boost(capture_image filesystem system)

#rosbuild_add_executable(opencv_performance_new_with_hough_line_transform common/src/performance_new_with_hough_line_transform.cpp)

#rosbuild_add_executable(create_template common/src/CreateTemplate.cpp)

#rosbuild_add_executable(CutTagFromBackground2 common/src/CutTagFromBackground2.cpp)

#rosbuild_add_executable(ExtractLBP common/src/ExtractLBP.cpp
#									common/src/lbp.cpp
#									common/src/histogram.cpp)
			
									
											
#rosbuild_add_executable(CreateSamplesForSingleCharacters common/src/CreateSamplesForSingleCharacters.cpp)

#	rosbuild_add_executable(RenameTrainingSamplesForSVM common/src/RenameTrainingSamplesForSVM.cpp)								