//This program is modified from original performance.cpp to give the performance of new cascade classifier and also save the detected text tags' infomation into a xml file like following formate.
//<?xml version="1.0" encoding="UTF-8"?>
//<tagset>
//  <image>
//    <imageName>images/image1.jpg</imageName>
//   <taggedRectangles>
//     <taggedRectangle x="1276" y="900" width="193" height="61" modelType="17"  />
//      <taggedRectangle x="348" y="844" width="197" height="105" modelType="3"  />
//      <taggedRectangle x="776" y="812" width="281" height="165" modelType="42"  />
//    </taggedRectangles>
//  </image>
//  <image>
//    <imageName>images/image2.jpg</imageName>
//    <taggedRectangles>
//      <taggedRectangle x="1036" y="972" width="75" height="29" modelType="1"  />
//      <taggedRectangle x="110" y="964" width="411" height="45" modelType="2"  />
//      <taggedRectangle x="1126" y="966" width="441" height="59" modelType="3"  />
//      <taggedRectangle x="604" y="964" width="349" height="53" modelType="2"  />
//      <taggedRectangle x="262" y="766" width="595" height="67" modelType="17"  />
//      <taggedRectangle x="948" y="766" width="461" height="71" modelType="36"  />
//      <taggedRectangle x="366" y="318" width="57" height="29" modelType="14"  />
//      <taggedRectangle x="598" y="844" width="411" height="89" modelType="11"  />
//      <taggedRectangle x="828" y="388" width="181" height="45" modelType="11"  />
//      <taggedRectangle x="1004" y="292" width="197" height="109" modelType="14"  />
//      <taggedRectangle x="844" y="8" width="581" height="233" modelType="31"  />
//    </taggedRectangles>
//  </image>
//</tagset>
//

#include "cv.h"
#include "highgui.h"

#include <cstdio>
#include <cmath>
#include <ctime>
#include <math.h>
#include "opencv2/objdetect/objdetect.hpp"
#include "opencv2/highgui/highgui.hpp"
#include "opencv2/imgproc/imgproc.hpp"

#include <iostream>
#include <stdio.h>

#ifndef PATH_MAX
#define PATH_MAX 512
#endif /* PATH_MAX */

/*typedef struct HidCascade {
 int size;
 int count;
 } HidCascade;
 */

typedef struct ObjectPos {
    float x;
    float y;
    float width;
    int found; /* for reference */
    int neghbors;
} ObjectPos;

using namespace std;
using namespace cv;

int main(int argc, char* argv[]) {
    int i, j;
    char* classifierdir = NULL;
    //char* samplesdir    = NULL;

    int saveDetected = 1;
    double scale_factor = 1.1;
    float maxSizeDiff = 1.5F;
    float maxPosDiff = 1.1F;

    /* number of stages. if <=0 all stages are used */
    //int nos = -1, nos0;
    int width = 71;
    int height = 18;

    int rocsize;

    FILE* info;
    FILE* resultados;
    FILE* detected_text_tags;
    char* infoname;
    char fullname[PATH_MAX];
    char detfilename[PATH_MAX];
    char* filename;
    char detname[] = "det-";

    CascadeClassifier cascade;

    double totaltime;

    if (!(resultados = fopen("resultados.txt", "w"))) {
        printf("Cannot create results file.\n");
        exit(-1);
    }

    if (!(detected_text_tags = fopen("detected_text_tags.xml", "w"))) {
        printf("Cannot create xml file.\n");
        exit(-1);
    }

    infoname = (char*) "";
    rocsize = 20;
    if (argc == 1) {
        printf("Usage: %s\n  -data <classifier_directory_name>\n"
                "  -info <collection_file_name>\n"
                "  [-maxSizeDiff <max_size_difference = %f>]\n"
                "  [-maxPosDiff <max_position_difference = %f>]\n"
                "  [-sf <scale_factor = %f>]\n"
                "  [-ni]\n"
                "  [-rs <roc_size = %d>]\n"
                "  [-w <sample_width = %d>]\n"
                "  [-h <sample_height = %d>]\n", argv[0], maxSizeDiff,
                maxPosDiff, scale_factor, rocsize, width, height);

        return 0;
    }

    for (i = 1; i < argc; i++) {
        if (!strcmp(argv[i], "-data")) {
            classifierdir = argv[++i];
        } else if (!strcmp(argv[i], "-info")) {
            infoname = argv[++i];
        } else if (!strcmp(argv[i], "-maxSizeDiff")) {
            maxSizeDiff = (float) atof(argv[++i]);
        } else if (!strcmp(argv[i], "-maxPosDiff")) {
            maxPosDiff = (float) atof(argv[++i]);
        } else if (!strcmp(argv[i], "-sf")) {
            scale_factor = atof(argv[++i]);
        } else if (!strcmp(argv[i], "-ni")) {
            saveDetected = 0;
        } else if (!strcmp(argv[i], "-rs")) {
            rocsize = atoi(argv[++i]);
        } else if (!strcmp(argv[i], "-w")) {
            width = atoi(argv[++i]);
        } else if (!strcmp(argv[i], "-h")) {
            height = atoi(argv[++i]);
        }
    }

    if (!cascade.load(classifierdir)) {
        printf("Unable to load classifier from %s\n", classifierdir);
        return 1;
    }

    strcpy(fullname, infoname);
    filename = strrchr(fullname, '\\');
    if (filename == NULL) {
        filename = strrchr(fullname, '/');
    }
    if (filename == NULL) {
        filename = fullname;
    } else {
        filename++;
    }

    info = fopen(infoname, "r");
    totaltime = 0.0;

    if (info != NULL) {

        //int x, y, width, height;
	int x, y;
        Mat img;
        int hits, missed, falseAlarms;
        int totalHits, totalMissed, totalFalseAlarms;
        int found;
        float distance;
	


        int refcount;
        ObjectPos* ref;
        int detcount;
        ObjectPos* det;
        int error = 0;

        int* pos;
        int* neg;

        pos = (int*) cvAlloc(rocsize * sizeof(*pos));
        neg = (int*) cvAlloc(rocsize * sizeof(*neg));
        for (i = 0; i < rocsize; i++) {
            pos[i] = neg[i] = 0;
        }

        printf("+================================+======+======+======+==========+==========+==========+==========+\n");
        printf("|            File Name           | Hits |Missed| False|retangle.x|retangle.y|retangle.w|retangle.h|\n");
        printf("+================================+======+======+======+==========+==========+==========+==========+\n");
        fprintf(resultados,
                "+===============================+======+======+======+==========+==========+==========+==========+\n");
        fprintf(resultados,
                "|            File Name          | Hits |Missed| False|retangle.x|retangle.y|retangle.w|retangle.h|\n");
        fprintf(resultados,
                "+===============================+======+======+======+==========+==========+==========+==========+\n");
        //fprintf (resultados, "%d\n",framesCnt);

	fprintf(detected_text_tags,"<?xml version=\"1.0\" encoding=\"UTF-8\"\?>\n<tagset>\n");

        totalHits = totalMissed = totalFalseAlarms = 0;
        while (!feof(info)) {
            fscanf(info, "%s %d", filename, &refcount);
            img = imread(fullname);

            if (!img.data) {
                cout << "ow" << endl;
                return -1;
            }
            ref = (ObjectPos*) cvAlloc(refcount * sizeof(*ref));
            for (i = 0; i < refcount; i++) {
                error = (fscanf(info, "%d %d %d %d", &x, &y, &width, &height)
                        != 4);
                if (error)
                    break;
                ref[i].x = 0.5F * width + x;
                ref[i].y = 0.5F * height + y;
                ref[i].width = sqrt(0.5F * (width * width + height * height));
                ref[i].found = 0;
                ref[i].neghbors = 0; //in the new cascade, where to get the neighbors?
            }

            vector<Rect> obj_detectados;
            Rect retang;
            std::vector<cv::Rect> rectangles_list;
            if (!error) {
                totaltime -= time(0);

                cascade.detectMultiScale(img, obj_detectados, scale_factor, 4, 0
                //|CV_HAAR_FIND_BIGGEST_OBJECT
                // |CV_HAAR_DO_ROUGH_SEARCH
                        | CV_HAAR_SCALE_IMAGE, cv::Size(35, 9));

                totaltime += time(0);

                if (obj_detectados.size() == 0) 
		{
                    detcount = 0;
               	} 
		else 
		{
                    detcount = obj_detectados.size();
                }

                if (obj_detectados.size() > 1)
                	for( unsigned int i = 0; i < obj_detectados.size(); i++ )
                	{
                		rectangles_list.push_back(obj_detectados[i]);
                	}
                	groupRectangles(rectangles_list,1,0.8);

                det = (detcount > 0) ?
                        ((ObjectPos*) cvAlloc(detcount * sizeof(*det))) : NULL;
                hits = missed = falseAlarms = 0;
		i =0;
		fprintf (detected_text_tags,"  <image>\n");
		fprintf (detected_text_tags,"    <imageName>tests/");
		fprintf (detected_text_tags,"%s", filename);
		fprintf (detected_text_tags,"</imageName>\n");
		fprintf (detected_text_tags,"    <taggedRectangles>\n");
		//fprintf (detected_text_tags,"%s \n", filename);
                for (vector<Rect>::const_iterator r = obj_detectados.begin(); r != obj_detectados.end(); r++, i++)
		{
                    Point r1, r2;

                    r1.x = (r->x);
                    r1.y = (r->y);
                    r2.x = (r->x + r->width);
                    r2.y = (r->y + r->height);

                    retang.x = r1.x;
                    retang.y = r1.y;
                    retang.width = abs(r2.x - r1.x);
                    retang.height = abs(r2.y - r1.y);

		    //write the geometry infomation of detected text tags into xml file.
		    //fprintf (detected_text_tags, "%d \n", retang.x);
		    //fprintf (detected_text_tags, "%d \n", retang.y);
		    //fprintf (detected_text_tags, "%d \n", retang.width);
		    //fprintf (detected_text_tags, "%d \n", retang.height);
		    fprintf (detected_text_tags, "      <taggedRectangle x=\"");
		    fprintf (detected_text_tags, "%d", retang.x);
		    fprintf (detected_text_tags, "\" y=\"");
		    fprintf (detected_text_tags, "%d", retang.y);
		    fprintf (detected_text_tags, "\" width=\"");
		    fprintf (detected_text_tags, "%d", retang.width);
		    fprintf (detected_text_tags, "\" height=\"");
		    fprintf (detected_text_tags, "%d", retang.height);
		    fprintf (detected_text_tags, "\" modelType=\"1\"  />\n");
		    if (rectangles_list.size()>0)
		    {
		    	//cout<<"rectangles_list size:"<< rectangles_list.size()<<endl;

		    	rectangle( img, cv::Point(rectangles_list[1].x,rectangles_list[1].y), cv::Point(rectangles_list[1].x + rectangles_list[1].width,rectangles_list[1].y+rectangles_list[1].height), cv::Scalar( 255, 255, 0 ) , 4 , 8 , 0 );
		    }
		    	//printf(filename, "retangle.x: ", retang.x, "retangle.y: ", retang.y, "retangle.w: ", retang.width, "retangle.h: ", retang.height);
			if (saveDetected)
			{
			rectangle(img, retang, Scalar(0, 0, 255), 1, CV_AA);
			}

			det[i].x = 0.5F*r->width + r->x;
			det[i].y = 0.5F*r->height + r->y;
			det[i].width = sqrt(0.5F * (r->width * r->width + r->height * r->height));
			det[i].neghbors = 2; // i don't know if it will work...
                     // det[i].neghbors = r.neighbors; --- how to do it in the new version??

                    found = 0;
                    for (j = 0; j < refcount; j++) {
                        distance = sqrtf( (det[i].x - ref[j].x) * (det[i].x - ref[j].x) +
                                (det[i].y - ref[j].y) * (det[i].y - ref[j].y) );
                        //cout << distance << endl;
                        if( (distance < ref[j].width * maxPosDiff) &&
                            (det[i].width > ref[j].width / maxSizeDiff) &&
                            (det[i].width < ref[j].width * maxSizeDiff) )
                        {
                            ref[j].found = 1;
                            ref[j].neghbors = MAX( ref[j].neghbors, det[i].neghbors );
                            found = 1;
                        }
                    }

                    if (!found) {
                        falseAlarms++;
                        neg[MIN(det[i].neghbors, rocsize - 1)]++;
                        //neg[MIN(0, rocsize - 1)]++;
                    }
                }
                //imshow("teste", img);
		fprintf (detected_text_tags,"    </taggedRectangles>\n");
		fprintf (detected_text_tags,"  </image>\n");
                if (saveDetected) {
                    strcpy(detfilename, detname);
                    strcat(detfilename, filename);
                    strcpy(filename, detfilename);
                    imwrite(fullname, img);
                    //cvvSaveImage(fullname, img);
                }

                for (j = 0; j < refcount; j++) {
                    if (ref[j].found) {
                        hits++;
                        //pos[MIN(0, rocsize - 1)]++;
                        pos[MIN(ref[j].neghbors, rocsize - 1)]++;
                    } else {
                        missed++;
                    }
                }

                totalHits += hits;
                totalMissed += missed;
                totalFalseAlarms += falseAlarms;
                printf("|%32.64s|%6d|%6d|%6d|\n", filename, hits, missed,
                        falseAlarms);
                //printf("+--------------------------------+------+------+------+\n");
                fprintf(resultados, "|%32.64s|%6d|%6d|%6d|%10d|%10d|%10d|%10d|\n", filename, hits, missed, falseAlarms, retang.x,retang.y,retang.width,retang.height);
                //fprintf(resultados,
                //      "+--------------------------------+------+------+------+\n");
                fflush(stdout);
                if (det) {
                    cvFree( &det);
                    det = NULL;
                }
            } /* if( !error ) */

            //char c = (char) waitKey(10);
            //              if (c == 27)
            //                  exit(0);

            cvFree( &ref);
        }
        fclose(info);

        printf("|%32.32s|%6d|%6d|%6d|\n", "Total", totalHits, totalMissed,
                totalFalseAlarms);
        fprintf(resultados, "|%32.32s|%6d|%6d|%6d|\n", "Total", totalHits,
                totalMissed, totalFalseAlarms);
        printf("+===============================+======+======+======+==========+==========+==========+==========+\n");
        fprintf(resultados,
               "+===============================+======+======+======+==========+==========+==========+==========+\n");
	fprintf (detected_text_tags, "</tagset>\n");
        //printf("Number of stages: %d\n", nos);
        //printf("Number of weak classifiers: %d\n", numclassifiers[nos - 1]);
        printf("Total time: %f\n", totaltime);
        fprintf(resultados, "Total time: %f\n", totaltime);

        /* print ROC to stdout */
        for (i = rocsize - 1; i > 0; i--) {
            pos[i - 1] += pos[i];
            neg[i - 1] += neg[i];
        }
        //fprintf(stderr, "%d\n", nos);
        for (i = 0; i < rocsize; i++) {
            fprintf(stderr, "\t%d\t%d\t%f\t%f\n", pos[i], neg[i],
                    ((float) pos[i]) / (totalHits + totalMissed),
                    ((float) neg[i]) / (totalHits + totalMissed));
        }

        cvFree( &pos);
        cvFree( &neg);
    }

    return 0;
}
