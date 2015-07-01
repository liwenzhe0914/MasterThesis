cmake_minimum_required(VERSION 2.8.3)
project(ftfootb_label_reading)

set(catkin_RUN_PACKAGES			# all ROS packages from package.xml (libopencv-dev is system dependency --> sudo apt-get install)
	roscpp
	roslib
	cv_bridge
	sensor_msgs
	image_transport
	message_filters
	tf
)

set(catkin_BUILD_PACKAGES 
	${catkin_RUN_PACKAGES}
	cmake_modules
)

## Find catkin macros and libraries
## if COMPONENTS list like find_package(catkin REQUIRED COMPONENTS xyz)
## is used, also find other catkin packages
## Creates a bunch of environment variables that may be used later in the script
## e.g. catkin_INCLUDE_DIRS

find_package(catkin REQUIRED COMPONENTS
	${catkin_BUILD_PACKAGES} # this makes ${catkin_LIBRARIES} include all libraries of ${catkin_BUILD_PACKAGES}
)

find_package(OpenCV REQUIRED)	# name identical to FindOpenCV.cmake in cmake_modules


###################################
## catkin specific configuration ##
###################################
## The catkin_package macro generates cmake config files for your package
## Declare things to be passed to dependent projects
## INCLUDE_DIRS: uncomment this if you package contains header files
## LIBRARIES: libraries you create in this project that dependent projects also need
## CATKIN_DEPENDS: catkin_packages dependent projects also need for build and run
## DEPENDS: system dependencies of this project that dependent projects also need
catkin_package(
INCLUDE_DIRS
	common/include
	ros/include
LIBRARIES
CATKIN_DEPENDS
	${catkin_RUN_PACKAGES}
DEPENDS
	OpenCV
)


###########
## Build ##
###########
## Specify additional locations of header files
## Your package locations should be listed before other locations
include_directories(
	common/include
	ros/include
	${catkin_INCLUDE_DIRS} 		# same name as find_package
	${OpenCV_INCLUDE_DIRS}
)

## Declare a cpp executable
add_executable(read_label	ros/src/read_label.cpp
							common/src/MatchTemplate.cpp
							common/src/FeatureRepresentation.cpp
							common/src/TextTagDetection.cpp
							common/src/LBPandHistogram.cpp
							common/src/TextTagLocalization.cpp)
target_link_libraries(read_label
	${catkin_LIBRARIES} # automatically links all catkin_BUILD_PACKAGES
	${OpenCV_LIBRARIES})
add_dependencies(read_label	${catkin_EXPORTED_TARGETS})


add_executable(match_template_node	common/src/MatchTemplateNode.cpp
									common/src/MatchTemplate.cpp)
target_link_libraries(match_template_node
	${catkin_LIBRARIES} # automatically links all catkin_BUILD_PACKAGES
	${OpenCV_LIBRARIES})
add_dependencies(match_template_node	${catkin_EXPORTED_TARGETS})
#rosbuild_link_boost(match_template_node filesystem system)


add_executable(extract_features_and_recognition
	common/src/ExtractFeaturesAndReconition.cpp
	common/src/MatchTemplate.cpp
	common/src/LBPandHistogram.cpp)
target_link_libraries(extract_features_and_recognition
	${catkin_LIBRARIES} # automatically links all catkin_BUILD_PACKAGES
	${OpenCV_LIBRARIES})
add_dependencies(extract_features_and_recognition	${catkin_EXPORTED_TARGETS})


add_executable(feature_representation_node
	common/src/FeatureRepresentationNode.cpp 
	common/src/LBPandHistogram.cpp
	common/src/FeatureRepresentation.cpp)
target_link_libraries(feature_representation_node
	${catkin_LIBRARIES} # automatically links all catkin_BUILD_PACKAGES
	${OpenCV_LIBRARIES})
add_dependencies(feature_representation_node	${catkin_EXPORTED_TARGETS})


add_executable(text_tag_detection_node
	common/src/TextTagDetectionNode.cpp
	common/src/TextTagDetection.cpp)
target_link_libraries(text_tag_detection_node
	${catkin_LIBRARIES} # automatically links all catkin_BUILD_PACKAGES
	${OpenCV_LIBRARIES})
add_dependencies(text_tag_detection_node	${catkin_EXPORTED_TARGETS})


#find_package(Boost 1.42.0)


############ obsolete ##############
#add_executable(text_tag_recogntion_test common/src/text_tag_recogntion_test.cpp 
#											common/src/LBPandHistogram.cpp
#											common/src/FeatureRepresentation.cpp
#											common/src/MatchTemplate.cpp)	
#add_executable(CutTagFromBackground2 common/src/CutTagFromBackground2.cpp)
#add_executable(detect_dashes_by_contour common/src/detect_dashes_by_contour.cpp)
#add_executable(resize common/src/resize.cpp)
#rosbuild_link_boost(resize filesystem system)
#add_executable(create_negatives_from_video common/src/CreateNegativesFromVideo.cpp)
#add_executable(puttext common/src/puttext.cpp)
#add_executable(combine_positives_and_negatives common/src/CombinePositivesAndNegatives.cpp)
#rosbuild_link_boost(resize filesystem system)
#add_executable(create_positive_images common/src/CreatePositiveImages.cpp)
#rosbuild_link_boost(create_positive_images filesystem system)
#add_executable(capture_image common/src/CaptureImage.cpp)
#rosbuild_link_boost(capture_image filesystem system)
#add_executable(opencv_performance_new_with_hough_line_transform common/src/performance_new_with_hough_line_transform.cpp)
#add_executable(create_template common/src/CreateTemplate.cpp)
#add_executable(CutTagFromBackground2 common/src/CutTagFromBackground2.cpp)
#add_executable(ExtractLBP common/src/ExtractLBP.cpp
#									common/src/lbp.cpp
#									common/src/histogram.cpp)
#add_executable(CreateSamplesForSingleCharacters common/src/CreateSamplesForSingleCharacters.cpp)
#add_executable(RenameTrainingSamplesForSVM common/src/RenameTrainingSamplesForSVM.cpp)


#############
## Install ##
#############
## Mark executables and/or libraries for installation
install(TARGETS read_label match_template_node extract_features_and_recognition feature_representation_node text_tag_detection_node
	ARCHIVE DESTINATION ${CATKIN_PACKAGE_LIB_DESTINATION}
	LIBRARY DESTINATION ${CATKIN_PACKAGE_LIB_DESTINATION}
	RUNTIME DESTINATION ${CATKIN_PACKAGE_BIN_DESTINATION}
)

install(DIRECTORY common/include/${PROJECT_NAME}/
	DESTINATION ${CATKIN_PACKAGE_INCLUDE_DESTINATION}
	FILES_MATCHING PATTERN "*.h"
	PATTERN ".svn" EXCLUDE
)

install(DIRECTORY ros/include/${PROJECT_NAME}/
	DESTINATION ${CATKIN_PACKAGE_INCLUDE_DESTINATION}
	FILES_MATCHING PATTERN "*.h"
	PATTERN ".svn" EXCLUDE
)

install(DIRECTORY ros/launch
	DESTINATION ${CATKIN_PACKAGE_SHARE_DESTINATION}/ros
	PATTERN ".svn" EXCLUDE
)

install(DIRECTORY common/files
	DESTINATION ${CATKIN_PACKAGE_SHARE_DESTINATION}/common
	PATTERN ".svn" EXCLUDE
)