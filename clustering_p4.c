#include <pif_plugin.h>
#include <nfp.h>
#include <mutexlv.h>
#include <stdint.h>
#include <nfp/me.h>
#include <nfp/mem_atomic.h>

#define CENTERS 2
#define POINTS 500
#define BUFFERS 2

#define UINT16 __declspec(emem shared scope(global) export) uint16_t

UINT16 distances[CENTERS][POINTS], current_iteration = 0, total_size = 0, flag_randomize = 1, vec_rand[CENTERS];
UINT16 received_points_index = 0, initial_sample = 0, additional_size = 0, centers_index = 0, randompoint = 0, randcount = 0;
UINT16 Xpoints[BUFFERS][POINTS], Ypoints[BUFFERS][POINTS], Xcenters[BUFFERS][CENTERS], Ycenters[BUFFERS][CENTERS], index_vector_receiving = 0, index_vector_clustering = 0;
UINT16 clusters[CENTERS][POINTS], shorter_distance[POINTS], count[CENTERS], num_packets = 0;
UINT16 i = 0, j = 0, saveX=0, saveY=0;
__declspec(emem shared scope(global) export) uint32_t sumX[CENTERS], sumY[CENTERS];

void resetVectors()
{
    for (j = 0; j < CENTERS; j++)
    {
        count[j] = 0;
        sumX[j] = 0;
        sumY[j] = 0;
    }
}

void pif_plugin_init()
{
    //
}

// this function is called only once
void pif_plugin_init_master()
{
    //
}

int ManhattanDistance(int XCenter, int XPoint, int YCenter, int YPoint)
{

    int X, Y;

    X = XCenter - XPoint;
    Y = YCenter - YPoint;

    X = (X > 0) ? X : -X;
    Y = (Y > 0) ? Y : -Y;

    return X + Y;
}

void ResetRandVector()
{
    for (i = 0; i < CENTERS; i++)
        vec_rand[i] = 0;
}

void CreateCluster(int current_size_copy)
{
    for (i = 0; i < CENTERS; i++)
    {
        //match the shorter distance in the array of distances and assign the point in the determined cluster group
        if (distances[i][current_size_copy] == shorter_distance[current_size_copy])
        {
            clusters[i][current_size_copy] = 1;
            //realizes the sum of Xs and Ys coordinates to calculate the new centroids
            sumX[i] += Xpoints[index_vector_clustering][current_size_copy];
            sumY[i] += Ypoints[index_vector_clustering][current_size_copy];
            count[i]++;
        }
        else
            clusters[i][current_size_copy] = -1;
    }
}

void GetShorterDistance(int current_size_copy)
{
    int save_shorter = 9999;

    //compare the distance of the current point and all the centroids to find the shorter distance
    for (i = 0; i < CENTERS; i++)
        if (distances[i][current_size_copy] < save_shorter)
        {
            shorter_distance[current_size_copy] = distances[i][current_size_copy];
            save_shorter = shorter_distance[current_size_copy];
        }
}

int pif_plugin_func(EXTRACTED_HEADERS_T *headers, MATCH_DATA_T *data)
{

    //extract the informations frrom the packet
    PIF_PLUGIN_ipv4_T *headerIpv4 = pif_plugin_hdr_get_ipv4(headers);
    PIF_PLUGIN_test_T *headerTest = pif_plugin_hdr_get_test(headers);

    //these are for showing the coords of one of the centroids as soon as the program has finished the calculations
    PIF_HEADER_SET_test___Xcentro(headerTest, saveX);
    PIF_HEADER_SET_test___Ycentro(headerTest, saveY);

    //get the number of iterations per packet and add to the number of done iterations
    additional_size = PIF_HEADER_GET_test___add(headerTest);
    total_size = current_iteration + additional_size;

    //defines the initial sample
    initial_sample = POINTS / 2;

    //add the received point to the array of points
    Xpoints[index_vector_receiving][received_points_index] = PIF_HEADER_GET_test___X(headerTest);
    Ypoints[index_vector_receiving][received_points_index] = PIF_HEADER_GET_test___Y(headerTest);
    received_points_index++;

    //checks the current index
    if (received_points_index == POINTS)
    {
        //change the buffer if the current is full
        index_vector_receiving++;
        received_points_index = 0;
    }

    //checks the current buffer
    if (index_vector_receiving == BUFFERS)
    {   
        index_vector_receiving = 0;
    }

    //assign centroids as random points if the initial sample of points is reached 
    if (received_points_index == initial_sample)
    {
        for (i = 0; i < CENTERS; i++)
        {
            //in this case, it's just assigning in order, due to not having the implementation of rand function yet
            Xcenters[index_vector_receiving][i] = Xpoints[index_vector_receiving][i];
            Ycenters[index_vector_receiving][i] = Ypoints[index_vector_receiving][i];

            // randompoint = rand() % initial_sample;
            // for (int j = 0; j < randcount; j++)
            //     while (randompoint == vec_rand[j])
            //         randompoint = rand() % initial_sample;
            // vec_rand[i] = randompoint;
            // randcount++;
            
        }
        flag_randomize = 0;
    }

    if (flag_randomize == 0)
        //realize the additional iterations per packet through the array.
        for (; current_iteration < total_size; current_iteration++)
        {
            if (current_iteration >= received_points_index && index_vector_clustering == index_vector_receiving) //prevents calculations while received_points_index is smaller than the current_iteration index
            {
                break;
            }
            if (current_iteration < POINTS)
            {
                //calculate the manhattan distance between the current point (current_iteration) and the current centroid (centers_index).
                distances[centers_index][current_iteration] = ManhattanDistance(Xcenters[index_vector_clustering][centers_index], Xpoints[index_vector_clustering][current_iteration], Ycenters[index_vector_clustering][centers_index], Ypoints[index_vector_clustering][current_iteration]);

                //set the current_distance to show if the program is working properly
                PIF_HEADER_SET_test___current_distance(headerTest, distances[centers_index][current_iteration]);
                
                //as the center_index is the last centroid, it can already start calculating the shorter distance and start clustering. 
                if (centers_index == CENTERS - 1) 
                {
                    //get the shorter distance of determined point to a centroid.
                    GetShorterDistance(current_iteration);
                    //assign the point to it's closer centroid group
                    CreateCluster(current_iteration);
                }
            }
        }

    //checks the number of iterations
    if (current_iteration >= POINTS)
    {
        //reset the vector and counter of random points
        //ResetRandVector();
        //randcount = 0;

        //increment the center index
        centers_index++;
        current_iteration = 0;
    }

    //checks the center index. If centers_index==CENTERS, the first clustering is complete.
    if (centers_index == CENTERS)
    {
        //stores the coords of one centroid
        saveX=(sumX[1]/count[1]);
        saveY=(sumY[1]/count[1]);
        //
        PIF_HEADER_SET_test___Xcentro(headerTest, saveX);
        PIF_HEADER_SET_test___Ycentro(headerTest, saveY);
        resetVectors();
        centers_index = 0;
        //start clustering points in the next buffer.
        index_vector_clustering++;
    }

    if (index_vector_clustering == BUFFERS)
    {
        index_vector_clustering = 0;
    }

 
    //update the current packet count 
    num_packets++;
    PIF_HEADER_SET_test___packets(headerTest, num_packets);


    return PIF_PLUGIN_RETURN_FORWARD;
}
