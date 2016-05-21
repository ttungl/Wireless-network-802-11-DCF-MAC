#include<iostream> 
#include<math.h>
#include <time.h>
using namespace std;


int numNodes;
int DIFS = 28; // us (=2*slotTime+SIFS) 
int SIFS = 10; // us
int maxFrameSize = 1500; // bytes
//------ in a node --
int numPktDrop[100];	// num packet dropped per each node i
int randSeed[100];		// random seed per each node i
double backoffNodes[100];	// backoff timer per each node i
int delayNode[100];		// delay on each node i

double x_coor[100];		// x coordinate
double y_coor[100];		// y coordinate
char SDpair[100];		// source destination pair
int nPackets[100];		// num packets per each node i 
double TX_range[100];		// transmission range of each node i
float errorChannelRand[100];// generate random probability error in channel

int neighborInUse[100];
int netLinks[100][100];

int nodeCW[100]; // us		// contention window
float totalDelay[100];//us	// total delay per each node i

int RTSsize = 15;			//bytes
int CTSsize = 15;			//bytes
int ACKsize = 15;			//bytes

int CTSrandomFlag[100];		
int ACKrandomFlag[100];

//-- inputs
int RTSCTSmode;
int nxn;
//-----------------

int dataRate = 54; // 11Mbps for 802.11b; 54Mbps for 802.11g
int slotTime = 9; // 20us(for 802.11b); 9us(for 802.11g)

int seedVal;
int numCollision = 0; // times
int CWmax = 1023*slotTime; // us
int CWmin = 15*slotTime; // us

int collisionDetected;
int CWmaxCounter=0;

float ProbGB; 
int errorChannelDetected =0; // error channel status; "0" means error-free; "1" means error in channel
int reTXcounter =0;
//------------------------
int numCollision1;
float totalDelay1[100];
int numPktDrop1[100];

//int randSeed[100];		// random seed per each node i
double backoffNodes1[100];	// backoff timer per each node i
int delayNode1[100];		// delay on each node i

int nPackets1[100];		// num packets per each node i 

float errorChannelRand1[100];// generate random probability error in channel

int nodeCW1[100];
//------------------------

int main ()
{
	printf("\n Tung Thanh Le \n CACS, UL Lafayette \n ttl8614@louisiana.edu \n Behavior of 802.11 DCF MAC (Phase 3)");
	
	printf("\n Input the network topology (NxN):");
	scanf_s("%d", &nxn);

	printf("Input the number of seeds:");
	scanf_s("%d", &seedVal);

	printf("Input the transition probability from good state to bad state \n The value of (P_GB) is between 0 and 1:");
	scanf_s("%f", &ProbGB);

	printf("\n Is RTS/CTS mode enabled? (Input 0/1 for No/Yes)");
	scanf_s("%d", &RTSCTSmode);


	// ====================================================
	numNodes = nxn*nxn; // num of nodes 
	int nn = nxn;
	// initial 
	for(int i=0; i<numNodes; i++){
		nodeCW[i] = CWmin;	// contention window
		nodeCW1[i] = CWmin;
		TX_range[i] = 150;	// transmission range in meter
		totalDelay[i] = 0;	// total delay per each node
		numPktDrop[i] = 0;	// num packet dropped
		//nPackets[i] = 1000; // total num packet per each node
		nPackets[i] = 1000; // total num packet per each node
		nPackets1[i] = 1000;
	}

	// construct the coordinator of the network topology
	int v=0, countY=0;
	for(int i=0; i<numNodes; i++){
		x_coor[i] = (i%nn)*145; // x-coordinate
		// y-coordinate
		if(countY < nn){
			y_coor[i] = v*145;
			countY = countY +1;
		} else {
			countY = 0;  // reset for a new row
			countY = countY +1;
			v = v +1;
			y_coor[i] = v*145;
		}
		// display the coordinator's information
		printf("\n node %d (x %f y %f), TX range %f \n", i, x_coor[i], y_coor[i], TX_range[i]);
	}

	
	// assign the source nodes and destination nodes for the network
	int numSrcNodes =0;
	for(int i=0; i<numNodes; i++){
		if(i%2==1){
			SDpair[i] = 'd'; // destination
		} else {
			SDpair[i] = 's'; // source
			numSrcNodes = numSrcNodes +1;
		}
	}

	// Initialize the values of neighborInUse
	for(int i=0; i<numNodes; i++){
		for(int j=0; j<numNodes; j++){
			neighborInUse[i] = 0;
			neighborInUse[j] = 0;
		}
	}

	// Find neighborhood
	double dx, dy, xydist;
	for(int i=0; i<numNodes; i++){
		for(int j=0; (j<numNodes) && (j!=i); j++){
			dx = x_coor[i] - x_coor[j];
			dy = y_coor[i] - y_coor[j];
			xydist = sqrt(dx*dx + dy*dy);

			// check TX range
			if(xydist < TX_range[i]){
				// check the node is in use
				if((neighborInUse[i] == 0)&&(neighborInUse[j] == 0)){
					netLinks[i][j] = 1; // link established
					neighborInUse[i] = 1;
					neighborInUse[j] = 1;
				} else {
					netLinks[i][j] = 0; // link not established
				}
			} else {
				netLinks[i][j] = 0; // link not established
			}

			// assign the last node as a source node if the numNodes is an odd number (e.g., 3x3)
			if(nn%2 != 0){
				if(neighborInUse[numNodes-1] == 0){
					netLinks[numNodes-2][numNodes-1] = 1;
				}
			}
		}
	}

	// check netLinks table
	for(int i=0; i<numNodes; i++){
		for(int j=0; j<numNodes; j++){
			cout<<netLinks[i][j];	
		}
		cout<< endl;
	}

	// random number generator
	srand(seedVal);
	for (int i=0; i<numNodes; i++) {
		randSeed[i] = rand();
		backoffNodes[i] = ((randSeed[i] % nodeCW[i])*(101/100));
		//printf("\n backoff %f", backoffNodes[i]);
	}

	
	// bubble sort on backoffNodes min-max
	for (int i=0; i<numNodes; i++) {
		for (int j=0; j<(numNodes-i-1); j++) {
			if (backoffNodes[j] > backoffNodes[j+1]) { // increasing order
				int swap = backoffNodes[j];
				backoffNodes[j] = backoffNodes[j+1];
				backoffNodes[j+1] = swap;
				
			}
		}
	}
	
	// bubble sort on backoffNodes min-max
	for (int i=0; i<numNodes; i++) {
		for (int j=0; j<(numNodes-i-1); j++) {
			if (backoffNodes1[j] < backoffNodes1[j+1]) { // decreasing order
				int swap = backoffNodes[j];
				backoffNodes1[j] = backoffNodes1[j+1];
				backoffNodes1[j+1] = swap;
				
			}
		}
	}
	
	// initial random value for detecting error in channel
	srand((unsigned)time(NULL));
	for (int i=0; i<numNodes; i++) {
		errorChannelRand[i] = ((float)rand() / (RAND_MAX));
		//printf("\n errorChannelRand %f", errorChannelRand[i]);
	}
	
	// initial random value for detecting error in channel
	srand((unsigned)time(NULL));
	for (int i=0; i<numNodes; i++) {
		errorChannelRand1[i] = ((float)rand() / (RAND_MAX));
		//printf("\n errorChannelRand %f", errorChannelRand[i]);
	}
	
	int ctsCollision[100];
	int ackCollision[100];
	int ctsPacketDropped[100];
	int ackPacketDropped[100];
	int totalPacketSourceNodes = nPackets[0]*numSrcNodes;
	int collisionFlag = 0;
	int pktDroppedFlag = 0;
	
	//--- RTS/CTS enable
	if(RTSCTSmode != 0){
		printf("\n RTS/CTS enable");
		while(totalPacketSourceNodes > 0){
			for(int i=0; i<numNodes; i+=2){		
				//if(SDpair[i] == 's'){
						if(errorChannelRand[i] > ProbGB){
								// error channel detected
								// update the total delay
								totalDelay[i] = totalDelay[i] + (maxFrameSize*8/dataRate); 
								
								srand((unsigned)time(NULL));
								for (int i=0; i<numNodes; i++) {
									errorChannelRand[i] = ((float)rand() / (RAND_MAX));
									printf("\n errorChannelRand detected %f", errorChannelRand[i]);
								}

								// error channel detected flag
								errorChannelDetected = 1;

								// retry to transmit frame i until reaching 4 times => frame is dropped!
								while((errorChannelRand[i] > ProbGB)&&(nPackets[i] !=0)){
									// update the total delay						
									totalDelay[i] = totalDelay[i] + ((maxFrameSize*8)/dataRate); // update the total delay
									
									// increase the counter for retrying
									reTXcounter = reTXcounter+1;	
									// check if over the upper-bound
									if((reTXcounter>=4)&&(nPackets[i] !=0)){ 
										// reset counter
										reTXcounter = 0;
										// reset errorChannelRand
										pktDroppedFlag = 1;
										// increase packet dropped
										numPktDrop[i] = numPktDrop[i] +1;
										//printf("\n Frame %d is dropped! ", i);
										nPackets[i] = nPackets[i] -1; // decrease a packet
										totalPacketSourceNodes = totalPacketSourceNodes -1; 
										//errorChannelRand[i] = 0; // skip this frame because it has already retried 4 times => frame dropped!
										pktDroppedFlag = 1;
									} else { 
										// update random number for retrying
										pktDroppedFlag = 0;
										srand((unsigned)time(NULL));
										for (int i=0; i<numNodes; i++) {
											errorChannelRand[i] = ((float)rand() / (RAND_MAX));
											//printf("errorChannelRand %f", errorChannelRand[i]);
										}
									}
								}

						} else {
								// no error channel
								//if((errorChannelRand[i] <= ProbGB)&&(pktDroppedFlag != 1)&&(totalPacketSourceNodes !=0)){								
									errorChannelDetected = 0;
									pktDroppedFlag = 0;
									for(int j= i+1; j<numNodes; j+=2){
										//if(SDpair[j] == 's'){
												// check if min backoff has no more than 1 ? 
												if((backoffNodes[i] != backoffNodes[j])&&(nPackets[i] !=0)&&(nPackets[j] !=0)){
													// no collision
													collisionDetected = 0;
													// Transmit RTS
													delayNode[i] = DIFS + backoffNodes[i]*slotTime + ((RTSsize*8)/dataRate); 
													totalDelay[i] = totalDelay[i] + delayNode[i]; // update the total delay
													printf("\n RTS transmitted at %f us", totalDelay[i]);
													
													// CTS randomization
													srand((unsigned)time(NULL));
													for (int i=0; i<numNodes; i++) {
														CTSrandomFlag[i] = ((float)rand() / (RAND_MAX));														
													}
													
													pktDroppedFlag = 0;
													ctsPacketDropped[i] = 0; // initial
													while(CTSrandomFlag[i] > ProbGB){
														// bad state
														collisionDetected =1;
														ctsCollision[i] = ctsCollision[i] +1;
														numCollision = numCollision + ctsCollision[i];
														// update the total delay
														totalDelay[i] = totalDelay[i] + ((CTSsize*8)/dataRate);

														// check if ctsCollision>7, drop this frame
														if((ctsCollision[i] > 7)&&(nPackets[i] !=0)){
															numPktDrop[i] = numPktDrop[i] +1;
															nPackets[i] = nPackets[i] -1;
															totalPacketSourceNodes = totalPacketSourceNodes -1;
															nodeCW[i] = CWmin;
															ctsCollision[i] = 0;
															CTSrandomFlag[i] = 0;
															ctsPacketDropped[i] = 1;
															pktDroppedFlag = 1;
														} else {
															pktDroppedFlag = 0;
															ctsPacketDropped[i] = 0;
															// CTS randomization
															srand((unsigned)time(NULL));
															for (int i=0; i<numNodes; i++) {
																CTSrandomFlag[i] = ((float)rand() / (RAND_MAX));														
															}
														}
													}
													
													if((CTSrandomFlag[i] <= ProbGB)&&(pktDroppedFlag != 1)&&(nPackets[i] !=0)){
														ctsPacketDropped[i] = 0;
														pktDroppedFlag = 0;
														// update the total delay
														totalDelay[i] = totalDelay[i] + 2*SIFS + ((maxFrameSize*8)/dataRate) + ((CTSsize*8)/dataRate);
														printf("\n CTS transmitted at %f us", totalDelay[i]);	
														
														// ACK randomization
														srand((unsigned)time(NULL));
														for (int i=0; i<numNodes; i++) {
															ACKrandomFlag[i] = ((float)rand() / (RAND_MAX));														
														}
														int ackPacketDropped[100];
														ackPacketDropped[i] = 0; // initial
														while(ACKrandomFlag[i] > ProbGB){
															// bad state
															collisionDetected =1;
															ackCollision[i] = ackCollision[i] +1;
															numCollision = numCollision + ackCollision[i];
															// update the total delay
															totalDelay[i] = totalDelay[i] + ((ACKsize*8)/dataRate);

															// check if ackCollision>7, drop this frame
															if((ackCollision[i] > 7)&&(nPackets[i] !=0)){
																numPktDrop[i] = numPktDrop[i] +1;
																nPackets[i] = nPackets[i] -1;
																totalPacketSourceNodes = totalPacketSourceNodes -1;
																nodeCW[i] = CWmin;
																ackCollision[i] = 0;
																ACKrandomFlag[i] = 0;
																ackPacketDropped[i] = 1;
																pktDroppedFlag = 1;
															} else {
																pktDroppedFlag = 0;
																ackPacketDropped[i] = 0;
																// randomize
																srand((unsigned)time(NULL));
																for (int i=0; i<numNodes; i++) {
																	ACKrandomFlag[i] = ((float)rand() / (RAND_MAX));														
																}
															}
														}
														
														if((ACKrandomFlag[i] <= ProbGB)&&(ackPacketDropped[i] != 1)){
														ackPacketDropped[i] = 0;
														pktDroppedFlag = 0;
														// update the total delay
														totalDelay[i] = totalDelay[i] + SIFS + ((ACKsize*8)/dataRate);
														printf("\n ACK transmitted at %f us", totalDelay[i]);
														nPackets[i] = nPackets[i] -1;
														totalPacketSourceNodes = totalPacketSourceNodes -1;
														
														printf("\n This frame is done!");	
														
														}
													} 
											} else { // same backoff timers, then check if within TX range
													float dx, dy, xydist;
													collisionDetected = 0;
													dx = x_coor[i] - x_coor[j];
													dy = y_coor[i] - y_coor[j];
													xydist = sqrt(dx*dx + dy*dy);
													if((xydist < TX_range[i])&&(nPackets[i] !=0)){
														// collision occurs
														// collision detected 
														collisionDetected = 1;
														// update the total delay
														totalDelay[i] = totalDelay[i] + ((maxFrameSize*8)/dataRate); 
														//
														
														numCollision = numCollision +1; // collision counter				
														// check CWmax
														if (nodeCW[i] >=CWmax) {
															nodeCW[i] = CWmax;
															printf("\n CWmax %d !", nodeCW[i]);
															CWmaxCounter = CWmaxCounter +1;
											
														} else {
															nodeCW[i] = nodeCW[i]*2; // double contention window
															printf("\n double CW !%d ", nodeCW[i]);
											
														}
										
														// network overheaded!
														if (CWmaxCounter >=3){
															printf("\n Network is overheaded %d !", nodeCW[i]);
														}
										
														// update random generator
														srand(seedVal);
														for (int i=0; i<numNodes; i++){
															randSeed[i] = rand();
														}
														// update backoff time
														backoffNodes[i] = (randSeed[i] % nodeCW[i]);
														backoffNodes[i+1] = (randSeed[i+1] % nodeCW[i]);
													}
											} // end if backoff
										//}  // end if sdpair
									} // end for
								//} // end if	
							} // end elseif

					
				
					// check if collision case
					if((collisionDetected != 0)&&(nPackets[i] !=0)){
						collisionDetected = 0;
						// bubble sort again
							for (int k=0; k<numNodes; k++) {
								for (int j=0; j<numNodes-k-1; j++){
									if(backoffNodes[j]>backoffNodes[j+1]){
										int swap = backoffNodes[j];
										backoffNodes[j] = backoffNodes[j+1];
										backoffNodes[j+1] = swap;
									}
								}
							}
							// Transmit RTS
							delayNode[i] = DIFS + backoffNodes[i]*slotTime + ((RTSsize*8)/dataRate); 
							totalDelay[i] = totalDelay[i] + delayNode[i]; // update the total delay
							
							// CTS randomization
							srand((unsigned)time(NULL));
							for (int i=0; i<numNodes; i++) {
								CTSrandomFlag[i] = ((float)rand() / (RAND_MAX));														
							}
							
							ctsPacketDropped[i] = 0; // initial
							while(CTSrandomFlag[i] > ProbGB){
								// bad state
								collisionDetected =1;
								ctsCollision[i] = ctsCollision[i] +1;
								numCollision = numCollision + ctsCollision[i];
								// update the total delay
								totalDelay[i] = totalDelay[i] + ((CTSsize*8)/dataRate);

								// check if ctsCollision>7, drop this frame
								if((ctsCollision[i] > 7)&&(nPackets[i] !=0)){
									numPktDrop[i] = numPktDrop[i] +1;
									nPackets[i] = nPackets[i] -1;
									totalPacketSourceNodes = totalPacketSourceNodes -1;
									nodeCW[i] = CWmin;
									ctsCollision[i] = 0;
									CTSrandomFlag[i] = 0;
									ctsPacketDropped[i] = 1;
								} else {
									ctsPacketDropped[i] = 0;
									// CTS randomization
									srand((unsigned)time(NULL));
									for (int i=0; i<numNodes; i++) {
										CTSrandomFlag[i] = ((float)rand() / (RAND_MAX));														
									}
								}
							}
							if((CTSrandomFlag[i] <= ProbGB)&&(ctsPacketDropped[i] != 1)){
								// update the total delay
								totalDelay[i] = totalDelay[i] + 2*SIFS + ((maxFrameSize*8)/dataRate) + ((CTSsize*8)/dataRate);
								
								// ACK randomization
								srand((unsigned)time(NULL));
								for (int i=0; i<numNodes; i++) {
									ACKrandomFlag[i] = ((float)rand() / (RAND_MAX));														
								}

								int ackPacketDropped[100];
								ackPacketDropped[i] = 0; // initial
								while(ACKrandomFlag[i] > ProbGB){
									// bad state
									collisionDetected =1;
									ackCollision[i] = ackCollision[i] +1;
									numCollision = numCollision + ackCollision[i];
									// update the total delay
									totalDelay[i] = totalDelay[i] + ((ACKsize*8)/dataRate);

									// check if ackCollision>7, drop this frame
									if((ackCollision[i] > 7)&&(nPackets[i] !=0)){
										numPktDrop[i] = numPktDrop[i] +1;
										nPackets[i] = nPackets[i] -1;
										totalPacketSourceNodes = totalPacketSourceNodes -1;
										nodeCW[i] = CWmin;
										ackCollision[i] = 0;
										ACKrandomFlag[i] = 0;
										ackPacketDropped[i] = 1;
									} else {
										ackPacketDropped[i] = 0;
										// CTS randomization
										srand((unsigned)time(NULL));
										for (int i=0; i<numNodes; i++) {
											CTSrandomFlag[i] = ((float)rand() / (RAND_MAX));														
										}
									}
								}
								
								if((ACKrandomFlag[i] <= ProbGB)&&(ackPacketDropped[i] != 1)){
								// update the total delay
								totalDelay[i] = totalDelay[i] + SIFS + ((ACKsize*8)/dataRate);
								
								nPackets[i] = nPackets[i] -1;
								totalPacketSourceNodes = totalPacketSourceNodes -1;
								
								printf("\n This collided frame is done!");	
								
								}
							}	
						
					} // end if collision=1
				//} // end if sdpair
			} // end for	 
		} //end while
	
	} else { // -- RTS/CTS disable
		printf("\n RTS/CTS disable");
   
		while(totalPacketSourceNodes > 0){
			for(int i=0; i<numNodes; i+=2){		
						// if(SDpair[i] == 's'){
								if(errorChannelRand1[i] > ProbGB){
										// error channel detected
										// update the total delay
										totalDelay1[i] = totalDelay1[i] + (maxFrameSize*8/dataRate); 
										
										srand((unsigned)time(NULL));
										for (int i=0; i<numNodes; i++) {
											errorChannelRand1[i] = ((float)rand() / (RAND_MAX));
											printf("\n errorChannelRand detected %f", errorChannelRand1[i]);
										}

										// error channel detected flag
										errorChannelDetected = 1;

										// retry to transmit frame i until reaching 4 times => frame is dropped!
										while((errorChannelRand1[i] > ProbGB)&&(nPackets1[i] !=0)){
											// update the total delay						
											totalDelay1[i] = totalDelay1[i] + ((maxFrameSize*8)/dataRate); // update the total delay
											
											// increase the counter for retrying
											reTXcounter = reTXcounter+1;	
											// check if over the upper-bound
											if((reTXcounter>=4)&&(nPackets1[i] !=0)){ 
												// reset counter
												reTXcounter = 0;
												// reset errorChannelRand
												pktDroppedFlag = 1;
												// increase packet dropped
												numPktDrop1[i] = numPktDrop1[i] +1;
												// printf("\n Frame %d is dropped! ", i);
												nPackets1[i] = nPackets1[i] -1; // decrease a packet
												totalPacketSourceNodes = totalPacketSourceNodes -1; 
												// errorChannelRand[i] = 0; // skip this frame because it has already retried 4 times => frame dropped!
												pktDroppedFlag = 1;
											} else { 
												// update random number for retrying
												pktDroppedFlag = 0;
												srand((unsigned)time(NULL));
												for (int i=0; i<numNodes; i++) {
													errorChannelRand1[i] = ((float)rand() / (RAND_MAX));
													// printf("errorChannelRand %f", errorChannelRand[i]);
												}
											}
										}

								} else {
										// no error channel
										// if((errorChannelRand[i] <= ProbGB)&&(pktDroppedFlag != 1)&&(totalPacketSourceNodes !=0)){								
											errorChannelDetected = 0;
											pktDroppedFlag = 0;
											for(int j= i+1; j<numNodes; j+=2){
												// if(SDpair[j] == 's'){
														// check if min backoff has no more than 1 ? 
													if((backoffNodes1[i] != backoffNodes1[j])&&(nPackets1[i] !=0)&&(nPackets1[j] !=0)){
														// no collision
														collisionDetected = 0;
														// Transmit data
														delayNode1[i] = DIFS + backoffNodes1[i]*slotTime + ((maxFrameSize*8)/dataRate) + SIFS + ((ACKsize*8)/dataRate); 
														totalDelay1[i] = totalDelay1[i] + delayNode1[i]; // update the total delay
														printf("\n Data+SIFS+ACK transmitted at %f us", totalDelay1[i]);
														nPackets1[i] = nPackets1[i] -1;
														totalPacketSourceNodes = totalPacketSourceNodes -1;
														nodeCW1[i] = CWmin;
														collisionDetected =0;
														 
													} else { // same backoff timers, then check if within TX range
															float dx, dy, xydist;
															collisionDetected = 0;
															dx = x_coor[i] - x_coor[j];
															dy = y_coor[i] - y_coor[j];
															xydist = sqrt(dx*dx + dy*dy);
															if((xydist < TX_range[i])&&(nPackets1[i] !=0)){
																// collision occurs
																// collision detected 
																collisionDetected = 1;
																// update the total delay
																totalDelay1[i] = totalDelay1[i] + ((maxFrameSize*8)/dataRate); 
																
																
																numCollision1 = numCollision1 +1; // collision counter				
																// check CWmax
																if (nodeCW1[i] >=CWmax) {
																	nodeCW1[i] = CWmax;
																	printf("\n CWmax %d !", nodeCW1[i]);
																	CWmaxCounter = CWmaxCounter +1;
													
																} else {
																	nodeCW1[i] = nodeCW1[i]*2; // double contention window
																	printf("\n double CW !%d ", nodeCW1[i]);
													
																}
												
																// network overheaded!
																if (CWmaxCounter >=3){
																	printf("\n Network is overheaded %d !", nodeCW1[i]);
																}
												
																// update random generator
																srand(seedVal);
																for (int i=0; i<numNodes; i++){
																	randSeed[i] = rand();
																}
																// update backoff time
																backoffNodes1[i] = (randSeed[i] % nodeCW1[i]);
																backoffNodes1[i+1] = (randSeed[i+1] % nodeCW1[i]);
															}
													} // end if backoff
												// }  // end if sdpair
											} // end for
										// } // end if	
									} // end elseif

							
						
							// check if collision case
							if((collisionDetected != 0)&&(nPackets1[i] !=0)){
								collisionDetected = 0;
								// bubble sort again
									for (int k=0; k<numNodes; k++) {
										for (int j=0; j<numNodes-k-1; j++){
											if(backoffNodes1[j]>backoffNodes1[j+1]){
												int swap1 = backoffNodes1[j];
												backoffNodes1[j] = backoffNodes1[j+1];
												backoffNodes1[j+1] = swap1;
											}
										}
									}
									// Transmit data
									delayNode1[i] = DIFS + backoffNodes1[i]*slotTime + ((maxFrameSize*8)/dataRate) + SIFS + ((ACKsize*8)/dataRate); 
									totalDelay1[i] = totalDelay1[i] + delayNode1[i]; // update the total delay
									printf("\n Collided Data+SIFS+ACK transmitted at %f us", totalDelay1[i]);
									nPackets1[i] = nPackets1[i] -1;
									totalPacketSourceNodes = totalPacketSourceNodes -1;
									nodeCW1[i] = CWmin;
									collisionDetected =0;
									
							}	
								
			} // end for
		} // end while
	} //end else
	



	
	
	printf("\n --------- RTS/CTS enable --------- ");
	printf("\n Number of Collision %d", numCollision);
	
	for(int i=0; i<numNodes; i+=2){ 
		printf("\n node %d: Total delay %f us", i, totalDelay[i]);
		
	}
	
	for(int i=0; i<numNodes; i+=2){ 
		printf("\n node %d: Number of packets dropped %d ", i, numPktDrop[i]);
		
	}

	printf("\n --------- RTS/CTS disable --------- ");
	printf("\n Number of Collision %d", numCollision1);
	
	for(int i=0; i<numNodes; i+=2){ 
		printf("\n node %d: Total delay %f us", i, totalDelay1[i]);
		
	}
	
	for(int i=0; i<numNodes; i+=2){ 
		printf("\n node %d: Number of packets dropped %d ", i, numPktDrop1[i]);
		
	}
		

	while(1){};

	
} // end main()
