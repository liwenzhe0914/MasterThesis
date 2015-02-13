#include "ftfootb_label_reading/ExtractFeatures.h"
#include "ftfootb_label_reading/MatchTemplate.h"


using namespace cv;
using namespace std;

String ocrRead(const cv::Mat& image)
{
	float score = 0;
	string output;
	cv::imwrite("patch.tiff", image);

	int result;

	std::string cmd = "tesseract patch.tiff patch"; // before: psm -8
	result = system(cmd.c_str());

	assert(!result);
	std::ifstream fin("patch.txt");
	std::string str;

	while (fin >> str)
	{
		std::cout << str << " ";
		std::string tempOutput;
		//hack: turned off
		//score += spellCheck(str, tempOutput, 2);
		//std::cout << " -->  \"" << tempOutput.substr(0, tempOutput.length() - 1) << "\" , score: " << score << std::endl;
		//output += tempOutput;
		output += str;
	}

	result = system("$(rm patch.txt patch.tiff)");
	return output;
}

Mat get_hogdescriptor_visual_image(Mat& origImg,
                                   vector<float>& descriptorValues,
                                   Size winSize,
                                   Size cellSize,
                                   int scaleFactor,
                                   double viz_factor)
{
    Mat visual_image;
    resize(origImg, visual_image, Size(origImg.cols*scaleFactor, origImg.rows*scaleFactor));

    int gradientBinSize = 9;
    // dividing 180° into 9 bins, how large (in rad) is one bin?
    float radRangeForOneBin = 3.14/(float)gradientBinSize;

    // prepare data structure: 9 orientation / gradient strenghts for each cell
	int cells_in_x_dir = winSize.width / cellSize.width;
    int cells_in_y_dir = winSize.height / cellSize.height;
//    int totalnrofcells = cells_in_x_dir * cells_in_y_dir;
    float*** gradientStrengths = new float**[cells_in_y_dir];
    int** cellUpdateCounter   = new int*[cells_in_y_dir];
    for (int y=0; y<cells_in_y_dir; y++)
    {
        gradientStrengths[y] = new float*[cells_in_x_dir];
        cellUpdateCounter[y] = new int[cells_in_x_dir];
        for (int x=0; x<cells_in_x_dir; x++)
        {
            gradientStrengths[y][x] = new float[gradientBinSize];
            cellUpdateCounter[y][x] = 0;

            for (int bin=0; bin<gradientBinSize; bin++)
                gradientStrengths[y][x][bin] = 0.0;
        }
    }

    // nr of blocks = nr of cells - 1
    // since there is a new block on each cell (overlapping blocks!) but the last one
    int blocks_in_x_dir = cells_in_x_dir - 1;
    int blocks_in_y_dir = cells_in_y_dir - 1;

    // compute gradient strengths per cell
    int descriptorDataIdx = 0;
//    int cellx = 0;
//    int celly = 0;

    for (int blockx=0; blockx<blocks_in_x_dir; blockx++)
    {
        for (int blocky=0; blocky<blocks_in_y_dir; blocky++)
        {
            // 4 cells per block ...
            for (int cellNr=0; cellNr<4; cellNr++)
            {
                // compute corresponding cell nr
                int cellx = blockx;
                int celly = blocky;
                if (cellNr==1) celly++;
                if (cellNr==2) cellx++;
                if (cellNr==3)
                {
                    cellx++;
                    celly++;
                }

                for (int bin=0; bin<gradientBinSize; bin++)
                {
                    float gradientStrength = descriptorValues[ descriptorDataIdx ];
                    descriptorDataIdx++;

                    gradientStrengths[celly][cellx][bin] += gradientStrength;

                } // for (all bins)


                // note: overlapping blocks lead to multiple updates of this sum!
                // we therefore keep track how often a cell was updated,
                // to compute average gradient strengths
                cellUpdateCounter[celly][cellx]++;

            } // for (all cells)


        } // for (all block x pos)
    } // for (all block y pos)


    // compute average gradient strengths
    for (int celly=0; celly<cells_in_y_dir; celly++)
    {
        for (int cellx=0; cellx<cells_in_x_dir; cellx++)
        {

            float NrUpdatesForThisCell = (float)cellUpdateCounter[celly][cellx];

            // compute average gradient strenghts for each gradient bin direction
            for (int bin=0; bin<gradientBinSize; bin++)
            {
                gradientStrengths[celly][cellx][bin] /= NrUpdatesForThisCell;
            }
        }
    }


    cout << "descriptorDataIdx = " << descriptorDataIdx << endl;

    // draw cells
    for (int celly=0; celly<cells_in_y_dir; celly++)
    {
        for (int cellx=0; cellx<cells_in_x_dir; cellx++)
        {
            int drawX = cellx * cellSize.width;
            int drawY = celly * cellSize.height;

            int mx = drawX + cellSize.width/2;
            int my = drawY + cellSize.height/2;

            rectangle(visual_image,
                      Point(drawX*scaleFactor,drawY*scaleFactor),
                      Point((drawX+cellSize.width)*scaleFactor,
                      (drawY+cellSize.height)*scaleFactor),
                      CV_RGB(100,100,100),
                      1);

            // draw in each cell all 9 gradient strengths
            for (int bin=0; bin<gradientBinSize; bin++)
            {
                float currentGradStrength = gradientStrengths[celly][cellx][bin];

                // no line to draw?
                if (currentGradStrength==0)
                    continue;

                float currRad = bin * radRangeForOneBin + radRangeForOneBin/2;

                float dirVecX = cos( currRad );
                float dirVecY = sin( currRad );
                float maxVecLen = cellSize.width/2;
                float scale = viz_factor; // just a visual_imagealization scale,
                                          // to see the lines better

                // compute line coordinates
                float x1 = mx - dirVecX * currentGradStrength * maxVecLen * scale;
                float y1 = my - dirVecY * currentGradStrength * maxVecLen * scale;
                float x2 = mx + dirVecX * currentGradStrength * maxVecLen * scale;
                float y2 = my + dirVecY * currentGradStrength * maxVecLen * scale;

                // draw gradient visual_imagealization
                line(visual_image,
                     Point(x1*scaleFactor,y1*scaleFactor),
                     Point(x2*scaleFactor,y2*scaleFactor),
                     CV_RGB(0,0,255),
                     1);

            } // for (all bins)

        } // for (cellx)
    } // for (celly)


    // don't forget to free memory allocated by helper data structures!
    for (int y=0; y<cells_in_y_dir; y++)
    {
      for (int x=0; x<cells_in_x_dir; x++)
      {
           delete[] gradientStrengths[y][x];
      }
      delete[] gradientStrengths[y];
      delete[] cellUpdateCounter[y];
    }
    delete[] gradientStrengths;
    delete[] cellUpdateCounter;

    return visual_image;

}

std::vector<std::string> folder_list(std::string path)
{
	std::vector<std::string> FolderNames;
	DIR *pDIR;
	struct dirent *entry;
	if ((pDIR = opendir(path.c_str())) != NULL)
	{
	  /* print all the files and directories within directory */
		while ((entry = readdir(pDIR)) != NULL)
		{
			std::string completeName = entry->d_name;
			std::string name_end = completeName.substr(completeName.find_last_of("/") + 1, completeName.length());
			std::string s = path;
			if (name_end !="." and name_end !="..")
				{
				s.append(completeName);
				FolderNames.push_back(s);
				}
		}
	}
	else
	{
		std::cout << "Error: Could not open path." << std::endl;
	}
return FolderNames;
}

std::vector<std::string> load_folder_of_image(std::string path)
{
	std::vector<std::string> ImageNames;
	DIR *pDIR;
	struct dirent *entry;
	if ((pDIR = opendir(path.c_str())) != NULL)
	{
		while ((entry = readdir(pDIR)) != NULL)
		{
			if (std::strcmp(entry->d_name, ".") != 0 && strcmp(entry->d_name, "..") != 0)
			{
				std::string completeName = entry->d_name;
				std::string imgend = completeName.substr(completeName.find_last_of(".") + 1, completeName.length() - completeName.find_last_of("."));
				if (std::strcmp(imgend.c_str(), "png") == 0 || std::strcmp(imgend.c_str(), "PNG") == 0
						|| std::strcmp(imgend.c_str(), "JPG") == 0 || std::strcmp(imgend.c_str(), "jpg") == 0
						|| std::strcmp(imgend.c_str(), "jpeg") == 0 || std::strcmp(imgend.c_str(), "Jpeg") == 0
						|| std::strcmp(imgend.c_str(), "bmp") == 0 || std::strcmp(imgend.c_str(), "BMP") == 0
						|| std::strcmp(imgend.c_str(), "TIFF") == 0 || std::strcmp(imgend.c_str(), "tiff") == 0
						|| std::strcmp(imgend.c_str(), "tif") == 0 || std::strcmp(imgend.c_str(), "TIF") == 0)
				{
					std::string s = path;
					if (s.at(s.length() - 1) != '/')
						s.append("/");
					s.append(entry->d_name);
					ImageNames.push_back(s);
//					std::cout << "imagename: " << s << std::endl;
				}
			}
		}
	}
	else
	{
		std::cout << "Error: Could not open path." << std::endl;
	}
return ImageNames;
}

std::vector<float> get_HOG_descriptor(cv::Mat img)
{
	//read image file
	Mat img_gray;

	//resizing
	resize(img, img, Size(64,64) ); //Size(64,48) ); //Size(32*2,16*2)); //Size(80,72) );
	//gray
	cvtColor(img, img_gray, CV_RGB2GRAY,CV_32FC1);
	HOGDescriptor d( Size(64,64), Size(16,16), Size(8,8), Size(8,8), 9);
	vector< float> descriptorsValues;
	vector< Point> locations;
	d.compute( img_gray, descriptorsValues, Size(0,0), Size(0,0), locations);
//	cout << "HOG descriptor size is " << d.getDescriptorSize() << endl;
//	cout << "img dimensions: " << img.cols << " width x " << img.rows << "height" << endl;
//	cout << "Found " << descriptorsValues.size() << " descriptor values" << endl;
//	cout << "Nr of locations specified : " << descriptorsValues.size() << endl;
//	cout<<"Got descriptors Values !"<<endl;
	return descriptorsValues;
}

std::vector<float> get_BRIEF_descriptor(cv::Mat img)
{
//	cout<<"get_BRIEF_descriptor"<<endl;
	cv::DenseFeatureDetector detector(12.0f, 1, 0.1f, 10);
	BriefDescriptorExtractor extractor(32);
	vector<KeyPoint> keyPoints;

	Mat descriptorsValues_Mat;
	//read image file
	Mat img_gray;

	//resizing
	resize(img, img, Size(57,57)); //Size(64,48) ); //Size(32*2,16*2)); //Size(80,72) );
//	resize(img, img, Size(320,320));
	//gray
	cvtColor(img, img_gray, CV_RGB2GRAY,CV_32FC1);
	KeyPoint pt(float(img_gray.cols/2),float(img_gray.rows/2),1,-1, 0, 0, -1);
	keyPoints.push_back(pt);

//	detector.detect(img_gray,keyPoints);
	extractor.compute(img_gray,keyPoints,descriptorsValues_Mat);
	std::vector<float> descriptorsValues;

	descriptorsValues.assign(descriptorsValues_Mat.datastart, descriptorsValues_Mat.dataend);

//	cout<<"Brief descriptorsValues mat size: "<<descriptorsValues_Mat.rows<<"x"<<descriptorsValues_Mat.cols<<endl;
////
//	cout<<"Brief descriptorsValues size: "<<descriptorsValues.size()<<endl;

	return descriptorsValues;
}

int convertLettersToASCII(string letter)
{
	int class_number;
	char label1 = letter.at(0);
	char label2 = letter.at(1);
	int class_number_temp = (int(label1)-64)*10 + (int(label2)-64);
	class_number=class_number_temp-10+(int(label1)-65)*16;
    return class_number;
}

string convertASCIIToLetters(int number)
{
	int ASCII_1st_letter;
	int remainder = number%26;
	std::string letter_class;

	if (remainder !=0)
	{
		ASCII_1st_letter = floor(number/26)+65;
		std::stringstream ss;
		ss<<char(ASCII_1st_letter)<<char(remainder+64);
		letter_class = ss.str();
		ss.str("");
	}
	else
	{
		ASCII_1st_letter = floor(number/26)+64;
		std::stringstream ss;
		ss<<char(ASCII_1st_letter)<<char(26+64);
		letter_class = ss.str();
		ss.str("");
	}
    return letter_class;
}
Mat get_HOG_descriptor_from_training_data(std::vector<std::string> FoldersFullNames,int HOG_numbers)
{
	char FullFileName[100];
	string foldersNames;
	Mat trainData,trainClasses;
	char *cstr;
	std::vector<std::string> ImageNames;
	vector< float> descriptorsValues;
	vector< Point> locations;
	int Classes;
	std::ofstream out_file;
	if (HOG_numbers == 1)
		out_file.open("common/files/HOGFeatures_numbers.txt");
	else
		out_file.open("common/files/HOGFeatures_letters.txt");

	for(unsigned int i=0; i< FoldersFullNames.size(); ++i)
	{
		string foldername_temp = FoldersFullNames.at(i);
//		cout<<"foldername_temp: "<<foldername_temp<<endl;
		foldersNames = foldername_temp.substr(foldername_temp.find_last_of("/") + 1, foldername_temp.length());
//		cout<<"folders: "<<foldersNames<<endl;
		//cout<<"path: "<<path<<endl;
		std::stringstream ss1;
		ss1<<foldername_temp<<"/"<<foldersNames<<"_";
		std::string FirstFileName = ss1.str();
		//cout<<"FirstFileName: "<<FirstFileName<<endl;
		ss1.str("");
		cstr = new char[FirstFileName.length() + 1];
		strcpy(cstr, FirstFileName.c_str());
		ImageNames = load_folder_of_image(foldername_temp);
		int FileNum = ImageNames.size();
		for(int k=0; k< FileNum; ++k)
		{
			if (HOG_numbers == 1)
			{
				out_file<<foldersNames<<" ";
			}
			else
			{
				int letter_class_number = convertLettersToASCII(foldersNames);
				out_file<<letter_class_number<<" ";
			}
			sprintf(FullFileName, "%s%d.png", cstr, k+1);
//			printf("%s\n", FullFileName);
			//cout<<"FullFileName:"<<FullFileName<<endl;

			Mat img = imread(FullFileName);
			descriptorsValues=get_HOG_descriptor(img);
			for(unsigned int j=1; j<= descriptorsValues.size(); ++j)
			{
				if (descriptorsValues[j-1] !=0)
				{
					//cout<<j<<":"<<descriptorsValues[j-1]<<" "<<" ";
					out_file<<j<<":"<<descriptorsValues[j-1]<<" ";
				}
			}

			// convert descriptorsValues vector to a row
			Mat row(1, descriptorsValues.size(),CV_32FC1);
			memcpy(row.data,descriptorsValues.data(),descriptorsValues.size()*sizeof(float));
			//cout<<"row="<<row<<endl;
			trainData.push_back(row);
			if (HOG_numbers == 1)
			{
				istringstream ( foldersNames ) >> Classes;
			}
			else
			{
				Classes = convertLettersToASCII(foldersNames);
			}
			trainClasses.push_back( Classes );
			out_file<<" # "<<foldersNames<<"\n";
		//printf("descriptor number =%d\n", descriptorsValues.size() );
		}
	}
	delete [] cstr;
//	cout << "trainData dimensions: " << trainData.cols << " width x " << trainData.rows << " height" << endl;
//	cout << "trainData dimensions: " << trainClasses.cols << " width x " << trainClasses.rows << " height" << endl;
	Mat trainData_trainClasses(trainData.rows,trainData.cols+trainClasses.cols,CV_32FC1,Scalar::all(0));
//	cout << "trainData dimensions: " << trainData_trainClasses.cols << " width x " << trainData_trainClasses.rows << " height" << endl;

///combine two Matrix -- trainData and trainClasses into one Matrix
	for (int i = 1;i<trainData.cols;i++)
	{
		trainData.col(i).copyTo(trainData_trainClasses.col(i));
	}
//	cout<<trainClasses.col(0)<<endl;
	trainClasses.col(0).copyTo(trainData_trainClasses.col(trainData.cols+trainClasses.cols-1));

	return trainData_trainClasses;//trainData_trainClasses contains trainData and trainClasses
}

Mat get_BRIEF_descriptor_from_training_data(std::vector<std::string> FoldersFullNames,int BRIEF_numbers)
{
	char FullFileName[100];
	string foldersNames;
	Mat trainData,trainClasses;
	char *cstr;
	std::vector<std::string> ImageNames;
	vector< float> descriptorsValues;
	vector< Point> locations;
	int Classes;
	std::ofstream out_file;
	if (BRIEF_numbers == 1)
		out_file.open("common/files/BRIEFF_numbers.txt");
	else
		out_file.open("common/files/BRIEFF_letters.txt");

	for(unsigned int i=0; i< FoldersFullNames.size(); ++i)
	{
		string foldername_temp = FoldersFullNames.at(i);
//		cout<<"foldername_temp: "<<foldername_temp<<endl;
		foldersNames = foldername_temp.substr(foldername_temp.find_last_of("/") + 1, foldername_temp.length());
//		cout<<"folders: "<<foldersNames<<endl;
		//cout<<"path: "<<path<<endl;
		std::stringstream ss1;
		ss1<<foldername_temp<<"/"<<foldersNames<<"_";
		std::string FirstFileName = ss1.str();
		//cout<<"FirstFileName: "<<FirstFileName<<endl;
		ss1.str("");
		cstr = new char[FirstFileName.length() + 1];
		strcpy(cstr, FirstFileName.c_str());
		ImageNames = load_folder_of_image(foldername_temp);
		int FileNum = ImageNames.size();
		for(int k=0; k< FileNum; ++k)
		{
			if (BRIEF_numbers == 1)
			{
				out_file<<foldersNames<<" ";
			}
			else
			{
				int letter_class_number = convertLettersToASCII(foldersNames);
				out_file<<letter_class_number<<" ";
			}
			sprintf(FullFileName, "%s%d.png", cstr, k+1);
//			printf("%s\n", FullFileName);
			//cout<<"FullFileName:"<<FullFileName<<endl;

			Mat img = imread(FullFileName);
			descriptorsValues=get_BRIEF_descriptor(img);
			for(unsigned int j=1; j<= descriptorsValues.size(); ++j)
			{
				if (descriptorsValues[j-1] !=0)
				{
					//cout<<j<<":"<<descriptorsValues[j-1]<<" "<<" ";
					out_file<<j<<":"<<descriptorsValues[j-1]<<" ";
				}
			}

			// convert descriptorsValues vector to a row
			Mat row(1, descriptorsValues.size(),CV_32FC1);
			memcpy(row.data,descriptorsValues.data(),descriptorsValues.size()*sizeof(float));
//			cout<<"row="<<row<<endl;
			trainData.push_back(row);
			if (BRIEF_numbers == 1)
			{
				istringstream ( foldersNames ) >> Classes;
			}
			else
			{
				Classes = convertLettersToASCII(foldersNames);
			}
			trainClasses.push_back( Classes );
			out_file<<" # "<<foldersNames<<"\n";
		//printf("descriptor number =%d\n", descriptorsValues.size() );
		}
	}
	delete [] cstr;
//	cout << "trainData dimensions: " << trainData.cols << " width x " << trainData.rows << " height" << endl;
//	cout << "trainData dimensions: " << trainClasses.cols << " width x " << trainClasses.rows << " height" << endl;
	Mat trainData_trainClasses(trainData.rows,trainData.cols+trainClasses.cols,CV_32FC1,Scalar::all(0));
//	cout << "trainData dimensions: " << trainData_trainClasses.cols << " width x " << trainData_trainClasses.rows << " height" << endl;

///combine two Matrix -- trainData and trainClasses into one Matrix
	for (int i = 1;i<trainData.cols;i++)
	{
		trainData.col(i).copyTo(trainData_trainClasses.col(i));
	}
//	cout<<trainClasses.col(0)<<endl;
	trainClasses.col(0).copyTo(trainData_trainClasses.col(trainData.cols+trainClasses.cols-1));
//	cout<<"trainData_trainClasses="<<endl<<trainData_trainClasses<<endl;

	return trainData_trainClasses;//trainData_trainClasses contains trainData and trainClasses
}


int main(int argc, char** argv)
	{
	bool training = 1;
	bool training_with_PCA =0;
	bool load_svm = 0;

	double start_time;
	double time_in_seconds;

	start_time = clock();
	MatchTemplate mt("/home/damon/git/care-o-bot/ftfootb/ftfootb_label_reading/common/files");

	time_in_seconds = (clock() - start_time) / (double)CLOCKS_PER_SEC;
	std::cout << "[" << time_in_seconds << " s] processing time for loading TM" << std::endl;
	//variables
	string foldersNames;
//	int letter_class,number_class;
	vector< float> descriptorsValues;
	vector< float> descriptorsValues_BRIEF;
	vector< Point> locations;

	std::string training_path = "/home/damon/training_samples_for_SVM/";
	std::string test_path = "/home/damon/MasterThesis/Database/TextRecognitionTestDatabase/";
	std::stringstream ss_numbers;
	ss_numbers<<training_path<<"numbers"<<"/";
	std::string path_numbers = ss_numbers.str();
//	cout<<"path_numbers: "<<path_numbers<<endl;
	ss_numbers.str("");
	std::stringstream ss_letters;
	ss_letters<<training_path<<"letters"<<"/";
	std::string path_letters = ss_letters.str();
//	cout<<"path_letters: "<<path_letters<<endl;
	ss_letters.str("");
//	std::string test_img = argv[1];
//	Mat testImg = imread (test_img);
	//load the folder of images for testing
	std::vector<std::string> testingImagesFullNames = load_folder_of_image(test_path);
//	cout<<"testingImagesFullNames.size:"<<testingImagesFullNames.size()<<endl;
	std::vector<std::string> groundTruth;
	for (unsigned int i=0;i<testingImagesFullNames.size();i++)
	{
		std::string temp_GT = testingImagesFullNames[i].substr(testingImagesFullNames[i].find_last_of("/") + 1,
				testingImagesFullNames[i].length() - testingImagesFullNames[i].find_last_of(".")+7);
//		cout<<"temp_GT: "<<temp_GT<<endl;
		groundTruth.push_back(temp_GT);
	}
//	std::string number_class_string = path.substr(path.find_last_of("/") + 1, path.length());
//	std::string letter_class_string = path.substr(path.find_last_of("/") + 1, path.length());
//	cout<<"class_string:"<<number_class_string<<endl;
//	cout<<"class_string:"<<letter_class_string<<endl;
//	istringstream ( number_class_string ) >> number_class;
//	istringstream ( letter_class_string ) >> letter_class;

	////////get trainData and trainClasses

	Mat numbers_trainData_and_trainClasses,letters_trainData_and_trainClasses,letters_trainData_and_trainClasses_BRIEF,numbers_trainData_and_trainClasses_BRIEF;

	if (training)
	{
		std::vector<std::string> numbersTrainingFoldersFullNames = folder_list(path_numbers);
		std::vector<std::string> lettersTrainingFoldersFullNames = folder_list(path_letters);
		cout<<"computing trainData_and_trainClasses from training dataset."<<endl;

		numbers_trainData_and_trainClasses=get_HOG_descriptor_from_training_data(numbersTrainingFoldersFullNames,1);
		letters_trainData_and_trainClasses=get_HOG_descriptor_from_training_data(lettersTrainingFoldersFullNames,0);
		letters_trainData_and_trainClasses_BRIEF=get_BRIEF_descriptor_from_training_data(lettersTrainingFoldersFullNames,0);
		numbers_trainData_and_trainClasses_BRIEF=get_BRIEF_descriptor_from_training_data(numbersTrainingFoldersFullNames,1);

		FileStorage fs1("common/files/numbers_trainData_and_trainClasses.yml", FileStorage::WRITE);
		fs1 << "numbers_trainData_and_trainClasses" << numbers_trainData_and_trainClasses;
		FileStorage fs2("common/files/letters_trainData_and_trainClasses.yml", FileStorage::WRITE);
		fs2 << "letters_trainData_and_trainClasses" << letters_trainData_and_trainClasses;
		FileStorage fs3("common/files/numbers_trainData_and_trainClasses_BRIEF.yml", FileStorage::WRITE);
		fs3 << "numbers_trainData_and_trainClasses_BRIEF" << numbers_trainData_and_trainClasses_BRIEF;
		FileStorage fs4("common/files/letters_trainData_and_trainClasses_BRIEF.yml", FileStorage::WRITE);
		fs4 << "letters_trainData_and_trainClasses_BRIEF" << letters_trainData_and_trainClasses_BRIEF;
		fs1.release();
		fs2.release();
		fs3.release();
		fs4.release();
	}
	else
	{
		//read features from files
		cout<<"Reading trainData_and_trainClasses from feature description files."<<endl;
		FileStorage fs1("common/files/numbers_trainData_and_trainClasses.yml", FileStorage::READ);
		fs1["numbers_trainData_and_trainClasses"] >> numbers_trainData_and_trainClasses;
		FileStorage fs2("common/files/letters_trainData_and_trainClasses.yml", FileStorage::READ);
		fs2["letters_trainData_and_trainClasses"] >> letters_trainData_and_trainClasses;
		FileStorage fs3("common/files/numbers_trainData_and_trainClasses_BRIEF.yml", FileStorage::READ);
		fs3["numbers_trainData_and_trainClasses_BRIEF"] >> numbers_trainData_and_trainClasses_BRIEF;
		FileStorage fs4("common/files/letters_trainData_and_trainClasses_BRIEF.yml", FileStorage::READ);
		fs4["letters_trainData_and_trainClasses_BRIEF"] >> letters_trainData_and_trainClasses_BRIEF;
		fs1.release();
		fs2.release();
		fs3.release();
		fs4.release();
	}
	// get trainData and trainClasses from Matrix
	Mat numbers_trainData (numbers_trainData_and_trainClasses,Rect(0,0,numbers_trainData_and_trainClasses.cols-1,numbers_trainData_and_trainClasses.rows));
	Mat letters_trainData (letters_trainData_and_trainClasses,Rect(0,0,letters_trainData_and_trainClasses.cols-1,letters_trainData_and_trainClasses.rows));
	Mat numbers_trainClasses=numbers_trainData_and_trainClasses.col(numbers_trainData_and_trainClasses.cols-1);
	Mat letters_trainClasses=letters_trainData_and_trainClasses.col(letters_trainData_and_trainClasses.cols-1);

	Mat numbers_trainData_BRIEF (numbers_trainData_and_trainClasses_BRIEF,Rect(0,0,numbers_trainData_and_trainClasses_BRIEF.cols-1,numbers_trainData_and_trainClasses_BRIEF.rows));
	Mat letters_trainData_BRIEF (letters_trainData_and_trainClasses_BRIEF,Rect(0,0,letters_trainData_and_trainClasses_BRIEF.cols-1,letters_trainData_and_trainClasses_BRIEF.rows));
	Mat numbers_trainClasses_BRIEF=numbers_trainData_and_trainClasses_BRIEF.col(numbers_trainData_and_trainClasses_BRIEF.cols-1);
	Mat letters_trainClasses_BRIEF=letters_trainData_and_trainClasses_BRIEF.col(letters_trainData_and_trainClasses_BRIEF.cols-1);

	Mat numbers_trainData_PCA,letters_trainData_PCA,numbers_trainData_BRIEF_PCA,letters_trainData_BRIEF_PCA;
	if (training_with_PCA)
	{
	/////////// PCA ////////
	cout<<"applying PCA..."<<endl;
	start_time = clock();


    PCA pca_numbers(numbers_trainData,Mat(),CV_PCA_DATA_AS_COL, 100);
    PCA pca_letters(letters_trainData,Mat(),CV_PCA_DATA_AS_COL, 100);
//    cout<<"PCA Mean:"<<endl;
//    cout<<pca.mean<<endl;

    pca_numbers.project(numbers_trainData,numbers_trainData_PCA);
    pca_letters.project(letters_trainData,letters_trainData_PCA);

    PCA pca_numbers_BRIEF(numbers_trainData_BRIEF,Mat(),CV_PCA_DATA_AS_COL, 100);
    PCA pca_letters_BRIEF(letters_trainData_BRIEF,Mat(),CV_PCA_DATA_AS_COL, 100);
//    cout<<"PCA Mean:"<<endl;
//    cout<<pca.mean<<endl;

    pca_numbers_BRIEF.project(numbers_trainData_BRIEF,numbers_trainData_BRIEF_PCA);
    pca_letters_BRIEF.project(letters_trainData_BRIEF,letters_trainData_BRIEF_PCA);

    cout<<"numbers_trainData_PCA size: "<<numbers_trainData_PCA.rows<<"x"<<numbers_trainData_PCA.cols<<endl;
    cout<<"letters_trainData_BRIEF size: "<<letters_trainData_BRIEF.rows<<"x"<<letters_trainData_BRIEF.cols<<endl;
    cout<<"letters_trainData_PCA size: "<<letters_trainData_PCA.rows<<"x"<<letters_trainData_PCA.cols<<endl;
  	cout<<"numbers_trainData_BRIEF size: "<<numbers_trainData_BRIEF.rows<<"x"<<numbers_trainData_BRIEF.cols<<endl;

  	cout<<"letters_trainData size: "<<letters_trainData.rows<<"x"<<letters_trainData.cols<<endl;
  	cout<<"letters_trainData_BRIEF_PCA size: "<<letters_trainData_BRIEF_PCA.rows<<"x"<<letters_trainData_BRIEF_PCA.cols<<endl;
  	cout<<"numbers_trainData size: "<<numbers_trainData.rows<<"x"<<numbers_trainData.cols<<endl;
  	cout<<"numbers_trainData_BRIEF_PCA size: "<<numbers_trainData_BRIEF_PCA.rows<<"x"<<numbers_trainData_BRIEF_PCA.cols<<endl;

	time_in_seconds = (clock() - start_time) / (double)CLOCKS_PER_SEC;
	std::cout << "[" << time_in_seconds << " s] processing time for PCA." << std::endl;

	}
	cout<<"finish processing training data"<<endl;

//	cout << "numbers_trainData dimensions: " << numbers_trainData.cols << " width x " << numbers_trainData.rows << " height" << endl;
//	cout << "numbers_trainClasses dimensions: " << numbers_trainClasses.cols << " width x " << numbers_trainClasses.rows << " height" << endl;
//	cout << "letters_trainData dimensions: " << letters_trainData.cols << " width x " << letters_trainData.rows << " height" << endl;
//	cout << "letters_trainClasses dimensions: " << letters_trainClasses.cols << " width x " << letters_trainClasses.rows << " height" << endl;

	CvSVM numbers_svm,letters_svm;
	CvSVM numbers_svm_BRIEF,letters_svm_BRIEF;

	start_time=clock();

	if(load_svm)
	{
		cout<<"loading SVM classifiers"<<endl;
		numbers_svm_BRIEF.load("common/files/numbers_svm_model_BRIEF.xml");
		letters_svm_BRIEF.load("common/files/letters_svm_model_BRIEF.xml");
		numbers_svm.load("common/files/numbers_svm_model_HOG.xml");
		letters_svm.load("common/files/letters_svm_model_HOG.xml");

		time_in_seconds = (clock() - start_time) / (double)CLOCKS_PER_SEC;
		std::cout << "[" << time_in_seconds << " s] processing time for loading 4 SVM." << std::endl;
	}
	else
	{
		cout<<"training SVM classifiers"<<endl;
		///train SVM with trainData and trainClasses


		CvSVMParams params;
		params.svm_type=SVM::C_SVC;
		params.C=1;
		params.kernel_type=SVM::LINEAR;
		params.term_crit=TermCriteria(CV_TERMCRIT_ITER, (int)1e7, 1e-6);
		start_time = clock();

		if (training_with_PCA)
		{
			cout<<"training SVM +HOG with PCA..."<<endl;
			numbers_svm.train(numbers_trainData_PCA,numbers_trainClasses, Mat(), Mat(),params);
			letters_svm.train(letters_trainData_PCA,letters_trainClasses, Mat(), Mat(),params);
		}
		else
		{
			cout<<"training SVM +HOG without PCA..."<<endl;
			numbers_svm.train(numbers_trainData,numbers_trainClasses, Mat(), Mat(),params);
			letters_svm.train(letters_trainData,letters_trainClasses, Mat(), Mat(),params);
		}
		time_in_seconds = (clock() - start_time) / (double)CLOCKS_PER_SEC;
		std::cout << "[" << time_in_seconds << " s] processing time for traing SVM" << std::endl;

		///train SVM with trainData and trainClasses with BRIEF


		start_time = clock();

		if (training_with_PCA)
		{
			cout<<"training SVM +BRIEF with PCA..."<<endl;
			numbers_svm_BRIEF.train(numbers_trainData_BRIEF_PCA,numbers_trainClasses_BRIEF, Mat(), Mat(),params);
			letters_svm_BRIEF.train(letters_trainData_BRIEF_PCA,letters_trainClasses_BRIEF, Mat(), Mat(),params);
		}
		else
		{
			cout<<"training SVM +BRIEF without PCA..."<<endl;
			numbers_svm_BRIEF.train(numbers_trainData_BRIEF,numbers_trainClasses_BRIEF, Mat(), Mat(),params);
			letters_svm_BRIEF.train(letters_trainData_BRIEF,letters_trainClasses_BRIEF, Mat(), Mat(),params);
		}



		time_in_seconds = (clock() - start_time) / (double)CLOCKS_PER_SEC;
		std::cout << "[" << time_in_seconds << " s] processing time for training SVM+BRIEF" << std::endl;

		numbers_svm_BRIEF.save("common/files/numbers_svm_model_BRIEF.xml");
		letters_svm_BRIEF.save("common/files/letters_svm_model_BRIEF.xml");
		numbers_svm.save("common/files/numbers_svm_model_HOG.xml");
		letters_svm.save("common/files/letters_svm_model_HOG.xml");
		std::cout << "SVM training model Saved" << std::endl;
	}

	KNearest numbers_knn(numbers_trainData,numbers_trainClasses);
	KNearest letters_knn(letters_trainData,letters_trainClasses);

	//strat training
	cout<<"training KNN classifiers"<<endl;
	start_time = clock();
	numbers_knn.train(numbers_trainData, numbers_trainClasses);
	letters_knn.train(letters_trainData, letters_trainClasses);
	time_in_seconds = (clock() - start_time) / (double)CLOCKS_PER_SEC;
	std::cout << "[" << time_in_seconds << " s] processing time for traing KNN" << std::endl;

	std::ofstream out_file_test;
	out_file_test.open("common/files/HOGFeatures_testing.txt");

	CvKNearest numbers_knn_BRIEF(numbers_trainData_BRIEF,numbers_trainClasses_BRIEF);
	CvKNearest letters_knn_BRIEF(letters_trainData_BRIEF,letters_trainClasses_BRIEF);

	//strat training
	cout<<"training KNN classifiers with BRIEF"<<endl;
	start_time = clock();
	numbers_knn_BRIEF.train(numbers_trainData_BRIEF, numbers_trainClasses_BRIEF);
	letters_knn_BRIEF.train(letters_trainData_BRIEF, letters_trainClasses_BRIEF);
	time_in_seconds = (clock() - start_time) / (double)CLOCKS_PER_SEC;

	///////start testing

	float accuracy_KNN_letters=0,accuracy_KNN_numbers=0,accuracy_SVM_letters=0,accuracy_SVM_numbers=0,
			accuracy_KNN_letters_BRIEF=0,accuracy_KNN_numbers_BRIEF=0,accuracy_SVM_letters_BRIEF=0,accuracy_SVM_numbers_BRIEF=0;
//	std::cout<<"im here"<<endl;
	for (unsigned int i=0;i<testingImagesFullNames.size();i++)
	{
		cout<<"test Nr."<<i+1<<endl;
		vector<string> portions_GT;
		portions_GT.push_back(groundTruth[i].substr(0,2));
		portions_GT.push_back(groundTruth[i].substr(3,2));
		portions_GT.push_back(groundTruth[i].substr(6,2));
		portions_GT.push_back(groundTruth[i].substr(9,2));
		cout << "Ground truth: "<<portions_GT[0]<<"-"<< portions_GT[1]<< "-"<<portions_GT[2]<< "-"<<portions_GT[3]<<endl;


		Mat testImg = imread (testingImagesFullNames[i],1);

		start_time=clock();
//		string output = ocrRead( testImg);
//		time_in_seconds = (clock() - start_time) / (double)CLOCKS_PER_SEC;
//		std::cout << "[" << time_in_seconds << " s] processing time for OCR" << std::endl;
//		cout<<output<<endl;

//		imshow("img",testImg);
//		waitKey(0);

	/////////	get HOG descriptors for testing

		std::vector<cv::Mat> image_portions;

		//	get letter portion

		cv::Mat letter_portion = testImg(cv::Rect(0,0, testImg.cols*0.25, testImg.rows));
//		cv::namedWindow( "Display subimages", CV_WINDOW_AUTOSIZE );
//		cv::imshow( "Display subimages", letter_portion );
//		cv::waitKey(0);
		image_portions.push_back(letter_portion);

		//	get numbers portions

		for (double x_min_ratio = 0.25; x_min_ratio < 1.0; x_min_ratio += 0.25)
		{
			cv::Mat number_portion = testImg(cv::Rect(testImg.cols*x_min_ratio,0, testImg.cols*0.25, testImg.rows));
//			cv::namedWindow( "Display subimages", CV_WINDOW_AUTOSIZE );
//			cv::imshow( "Display subimages", number_portion );
//			cv::waitKey(0);
			image_portions.push_back(number_portion);
		}

		descriptorsValues=get_HOG_descriptor(image_portions[0]);
		descriptorsValues_BRIEF=get_BRIEF_descriptor(image_portions[0]);

		for(unsigned int i=1; i<= descriptorsValues.size(); ++i)
		{
			if (descriptorsValues[i-1] !=0)
			{
				out_file_test<<i<<":"<<descriptorsValues[i-1]<<" ";
			}
		}
		out_file_test<<"\n";

		for(unsigned int i=1; i<= 3; ++i)
		{
			descriptorsValues=get_HOG_descriptor(image_portions[i]);
			descriptorsValues_BRIEF=get_BRIEF_descriptor(image_portions[i]);
			for(unsigned int j=1; j<= descriptorsValues.size(); ++j)
			{
				if (descriptorsValues[j-1] !=0)
				{
					out_file_test<<j<<":"<<descriptorsValues[j-1]<<" ";
				}
			}
			out_file_test<<"\n";
		}

		//write descritorValues of 4 portions(4 row matrix)
		Mat test_descriptorsValues;
		Mat row_temp(1, descriptorsValues.size(),CV_32FC1);
		Mat test_descriptorsValues_BRIEF;
		Mat row_temp_BRIEF(1, descriptorsValues_BRIEF.size(),CV_32FC1);


		// write text portions to descriptorsValues Matrix
		vector< float> test_descriptorsValues_temp;
		test_descriptorsValues_temp = get_HOG_descriptor(image_portions[0]);
		memcpy(row_temp.data,test_descriptorsValues_temp.data(),test_descriptorsValues_temp.size()*sizeof(float));//convert descriptorsValues(type vector) to row matrix
		test_descriptorsValues.push_back(row_temp);

		vector< float> test_descriptorsValues_temp_BRIEF;
		test_descriptorsValues_temp_BRIEF = get_BRIEF_descriptor(image_portions[0]);
		memcpy(row_temp_BRIEF.data,test_descriptorsValues_temp_BRIEF.data(),test_descriptorsValues_temp_BRIEF.size()*sizeof(float));//convert descriptorsValues(type vector) to row matrix
		test_descriptorsValues_BRIEF.push_back(row_temp_BRIEF);

		for (unsigned int j = 1;j<4;j++)
		{
			test_descriptorsValues_temp = get_HOG_descriptor(image_portions[j]);
			test_descriptorsValues_temp_BRIEF = get_BRIEF_descriptor(image_portions[j]);
			memcpy(row_temp.data,test_descriptorsValues_temp.data(),test_descriptorsValues_temp.size()*sizeof(float));
			memcpy(row_temp_BRIEF.data,test_descriptorsValues_temp_BRIEF.data(),test_descriptorsValues_temp_BRIEF.size()*sizeof(float));
			test_descriptorsValues.push_back(row_temp);
			test_descriptorsValues_BRIEF.push_back(row_temp_BRIEF);
		}

		//Build result function -- knn.find_nearest
		Mat results(1,1,CV_32FC1);
		Mat neighbourResponses = Mat::ones(1,10,CV_32FC1);
		Mat dist = Mat::ones(1, 10, CV_32FC1);

		Mat results_BRIEF(1,1,CV_32FC1);
		Mat neighbourResponses_BRIEF = Mat::ones(1,10,CV_32FC1);
		Mat dist_BRIEF = Mat::ones(1, 10, CV_32FC1);

		vector<int> text_label_result_KNN_int;//important:need to convert the 1st element from class number to letter
		vector<int> text_label_result_SVM_int_BRIEF;
		vector<int> text_label_result_KNN_int_BRIEF;
		vector<int> text_label_result_SVM_int;
		float response_BRIEF = letters_svm_BRIEF.predict(test_descriptorsValues_BRIEF.row(0),test_descriptorsValues_BRIEF.row(0).cols);
		text_label_result_SVM_int_BRIEF.push_back(response_BRIEF);

		letters_knn.find_nearest(test_descriptorsValues.row(0),5,results, neighbourResponses, dist);
		text_label_result_KNN_int.push_back(results.at<float>(0,0));

		letters_knn_BRIEF.find_nearest(test_descriptorsValues_BRIEF.row(0),5,results_BRIEF, neighbourResponses_BRIEF, dist_BRIEF);
		text_label_result_KNN_int_BRIEF.push_back(results_BRIEF.at<float>(0,0));

		float response = letters_svm.predict(test_descriptorsValues.row(0),test_descriptorsValues.row(0).cols);
		text_label_result_SVM_int.push_back(response);

		for (unsigned int j =1;j<4;j++)
		{
			numbers_knn.find_nearest(test_descriptorsValues.row(j),5,results, neighbourResponses, dist);
			text_label_result_KNN_int.push_back(results.at<float>(0,0));

			numbers_knn_BRIEF.find_nearest(test_descriptorsValues_BRIEF.row(j),5,results_BRIEF, neighbourResponses_BRIEF, dist_BRIEF);
			text_label_result_KNN_int_BRIEF.push_back(results_BRIEF.at<float>(0,0));

			float response = numbers_svm.predict(test_descriptorsValues.row(j),test_descriptorsValues.row(j).cols);
			text_label_result_SVM_int.push_back(response);

			float response_BRIEF = numbers_svm_BRIEF.predict(test_descriptorsValues_BRIEF.row(j),test_descriptorsValues_BRIEF.row(j).cols);
			text_label_result_SVM_int_BRIEF.push_back(response_BRIEF);
		}

		vector<string> text_label_result_KNN_string;
		vector<string> text_label_result_KNN_string_BRIEF;
		vector<string> text_label_result_SVM_string;
		vector<string> text_label_result_SVM_string_BRIEF;
		text_label_result_KNN_string.push_back(convertASCIIToLetters(text_label_result_KNN_int[0]));
		text_label_result_KNN_string_BRIEF.push_back(convertASCIIToLetters(text_label_result_KNN_int_BRIEF[0]));
		text_label_result_SVM_string.push_back(convertASCIIToLetters(text_label_result_SVM_int[0]));
		text_label_result_SVM_string_BRIEF.push_back(convertASCIIToLetters(text_label_result_SVM_int_BRIEF[0]));

		if (text_label_result_KNN_string[0].compare(portions_GT[0])==0)
			{
			accuracy_KNN_letters=accuracy_KNN_letters+1;
			}
		else
			1;
//			cout<<"accuracy_KNN_letters miss!"<<endl;

		if (text_label_result_KNN_string_BRIEF[0].compare(portions_GT[0])==0)
			{
			accuracy_KNN_letters_BRIEF=accuracy_KNN_letters_BRIEF+1;
			}
		else
			1;
//			cout<<"accuracy_KNN_letters_BRIEF miss!"<<endl;

		if (text_label_result_SVM_string[0].compare(portions_GT[0])==0)
			{
			accuracy_SVM_letters=accuracy_SVM_letters+1;
			}
		else
			1;
//			cout<<"accuracy_SVM_letters miss!"<<endl;

		if (text_label_result_SVM_string_BRIEF[0].compare(portions_GT[0])==0)
			{
			accuracy_SVM_letters_BRIEF=accuracy_SVM_letters_BRIEF+1;
			}
		else
			1;
//			cout<<"accuracy_SVM_letters_BRIEF miss!"<<endl;

		for (unsigned int j = 1;j<text_label_result_KNN_int.size();j++)
		{
			ostringstream convert;   // stream used for the conversion
			convert << text_label_result_KNN_int[j];      // insert the textual representation of 'Number' in the characters in the stream

			string temp = convert.str(); // set 'Result' to the contents of the stream
			text_label_result_KNN_string.push_back(temp);
		}


		for (unsigned int j = 1;j<text_label_result_KNN_int_BRIEF.size();j++)
		{
			ostringstream convert;   // stream used for the conversion
			convert << text_label_result_KNN_int_BRIEF[j];      // insert the textual representation of 'Number' in the characters in the stream

			string temp = convert.str(); // set 'Result' to the contents of the stream
			text_label_result_KNN_string_BRIEF.push_back(temp);
		}

		for (unsigned int j = 1;j<text_label_result_SVM_int.size();j++)
		{
			ostringstream convert;   // stream used for the conversion
			convert << text_label_result_SVM_int[j];// insert the textual representation of 'Number' in the characters in the stream

			string temp = convert.str(); // set 'Result' to the contents of the stream
			text_label_result_SVM_string.push_back(temp);
		}

		for (unsigned int j = 1;j<text_label_result_SVM_int_BRIEF.size();j++)
		{
			ostringstream convert;   // stream used for the conversion
			convert << text_label_result_SVM_int_BRIEF[j];// insert the textual representation of 'Number' in the characters in the stream

			string temp = convert.str(); // set 'Result' to the contents of the stream
			text_label_result_SVM_string_BRIEF.push_back(temp);
		}

		for (unsigned int j = 1;j<text_label_result_KNN_int.size();j++)
		{
			if (text_label_result_KNN_string[j].compare(portions_GT[j])==0)
				{
				accuracy_KNN_numbers=accuracy_KNN_numbers+1;
				}
			else
				1;
//				cout<<"accuracy_KNN_numbers miss!"<<endl;

			if (text_label_result_SVM_string[j].compare(portions_GT[j])==0)
				{
				accuracy_SVM_numbers=accuracy_SVM_numbers+1;
				}
			else
				1;
//				cout<<"accuracy_SVM_numbers miss!"<<endl;

			if (text_label_result_KNN_string_BRIEF[j].compare(portions_GT[j])==0)
				{
				accuracy_KNN_numbers_BRIEF=accuracy_KNN_numbers_BRIEF+1;
				}
			else
				1;
//				cout<<"accuracy_KNN_numbers_BRIEF miss!"<<endl;

			if (text_label_result_SVM_string_BRIEF[j].compare(portions_GT[j])==0)
				{
				accuracy_SVM_numbers_BRIEF=accuracy_SVM_numbers_BRIEF+1;
				}
			else
				1;
//				cout<<"accuracy_SVM_numbers_BRIEF miss!"<<endl;
		}

		cout << "From KNN, got label = "<< convertASCIIToLetters(text_label_result_KNN_int[0])<<"-"<<text_label_result_KNN_int[1]
										<<"-"<<text_label_result_KNN_int[2]<<"-"<<text_label_result_KNN_int[3]<< endl;
		cout << "From SVM, got label = "<< convertASCIIToLetters(text_label_result_SVM_int[0])<<"-"<<text_label_result_SVM_int[1]
										<<"-"<<text_label_result_SVM_int[2]<<"-"<<text_label_result_SVM_int[3]<< endl;
		cout << "From KNN with BRIEF, got label = "<< convertASCIIToLetters(text_label_result_KNN_int_BRIEF[0])<<"-"<<text_label_result_KNN_int_BRIEF[1]
										<<"-"<<text_label_result_KNN_int_BRIEF[2]<<"-"<<text_label_result_KNN_int_BRIEF[3]<< endl;
		cout << "From SVM with BRIEF, got label = "<< convertASCIIToLetters(text_label_result_SVM_int_BRIEF[0])<<"-"<<text_label_result_SVM_int_BRIEF[1]
												<<"-"<<text_label_result_SVM_int_BRIEF[2]<<"-"<<text_label_result_SVM_int_BRIEF[3]<< endl;
//		imshow("test_img",testImg);
//		waitKey(0);
		start_time = clock();
		std::string tag_label;
		cv::Mat gray_image;
		cv::cvtColor(testImg, gray_image, CV_BGR2GRAY);
		mt.read_tag(gray_image, tag_label);

//		time_in_seconds = (clock() - start_time) / (double)CLOCKS_PER_SEC;
//		std::cout << "Template matching : [" << time_in_seconds << " s] processing time" << std::endl;

		std::cout << "The text tag: " << tag_label << "." << std::endl;


		text_label_result_KNN_string.clear();
		text_label_result_SVM_string.clear();
	}
	cout << "HOG+KNN accuracy for letters:"<< accuracy_KNN_letters/50<<endl;
	cout << "HOG+SVM accuracy for letters:"<< accuracy_SVM_letters/50<<endl;
	cout << "HOG+KNN accuracy for numbers:"<< accuracy_KNN_numbers/150<<endl;
	cout << "HOG+SVM accuracy for numbers:"<< accuracy_SVM_numbers/150<<endl;
	cout << "BRIEF+KNN accuracy for letters:"<< accuracy_KNN_letters_BRIEF/50<<endl;
	cout << "BRIEF+KNN accuracy for numbers:"<< accuracy_KNN_numbers_BRIEF/150<<endl;
	cout << "BRIEF+SVM accuracy for letters:"<< accuracy_SVM_letters_BRIEF/50<<endl;
	cout << "BRIEF+SVM accuracy for numbers:"<< accuracy_SVM_numbers_BRIEF/150<<endl;
}
