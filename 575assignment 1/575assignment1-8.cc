#include<iostream> 
#include<math.h>

int numNodes;
int DIFS = 28; // us (=2*slotTime+SIFS) 
int SIFS = 10; // us
int maxFrameSize = 1500; // bytes
int dataRate = 54; // 11Mbps for 802.11b; 54Mbps for 802.11g
int slotTime = 9; // 20us(for 802.11b); 9us(for 802.11g)
int randSeed[50]; // random seed for nodes
int backoffNodes[50];
int delayNode[50]; // delay on each node
int Global_Timer=0; 
int seedVal;
int numCollision =0; // times
int CWmax = 1023*slotTime; // us
int CWmin = 15*slotTime; // us
int CW; // us
int collisionDetected;
int CWmaxCounter =0;

int main ()
{
	printf("\n Tung Thanh Le \n CACS, UL Lafayette \n ttl8614@louisiana.edu \n Behavior of 802.11 DCF MAC ");
	
	printf("\n Input the number of nodes (max.50):");
	scanf("%d", &numNodes);

	printf("Input the number of seeds:");
	scanf("%d", &seedVal);
		
	// ====================================================
	
	CW = CWmin; // initial contention window CW
	// random number generator
	srand(seedVal);
	
	for (int i=0; i<numNodes; i++) {
		randSeed[i] = rand();
		backoffNodes[i] = (randSeed[i] % CW);
		//printf("%d _ ", backoffNodes[i]);
		//printf("\n randSeed %d " , randSeed[i]);
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
	// check sort
	for (int i=0; i<numNodes; i++) {
		printf(" \n check sort 1 %d " , backoffNodes[i]);
	}
	
	int collisionFlag =0;
	// detect if collision
	for(int i=0; i<numNodes; i++){		
			// check if min backoff has no more than 1 ? 
			if(backoffNodes[i] != backoffNodes[i+1]){
				// no collision
				int frameData;
				collisionDetected = 0;
				delayNode[i] = DIFS + backoffNodes[i] + (maxFrameSize*8/dataRate); // delay time for node i transmits successfully.
				frameData = delayNode[i];
				Global_Timer = Global_Timer + delayNode[i]; // update the total delay
				// being transmitting
				while (frameData!=0) {
					frameData = frameData -1;
				}	
				// note that frame is transmitted
				if (frameData ==0){
					printf("\n Frame is transmitted!");
					
				} 
				
			} else { 
				// collision detected 
				// wait for a frame 
				int frameWait = (maxFrameSize*8/dataRate);
				while (frameWait!=0) {
					frameWait = frameWait -1;
				}
				Global_Timer = Global_Timer + frameWait; // update the total delay
				//
				collisionDetected = 1;
				numCollision = numCollision +1; // collision counter				
				// check CWmax
				if (CW>=CWmax) {
					CW = CWmax;
					printf("\n CWmax %d !", CW);
					CWmaxCounter = CWmaxCounter +1;
					
				} else {
					CW = CW*2; // double contention window
					printf("\n double CW !%d ", CW);
					
				}
				
				// network overheaded!
				if (CWmaxCounter >=3){
					printf("\n Network is overheaded %d !", CW);
				}
				
				
				// update random generator
				srand(seedVal);
				for (int i=0; i<numNodes; i++){
					randSeed[i] = rand();
				}
				// update backoff time
				backoffNodes[i] = (randSeed[i] % CW);
				backoffNodes[i+1] = (randSeed[i+1] % CW);
			}
			// check if collision case
			if(collisionDetected ==1){
				// bubble sort again
				int frameDataCollision;
				
				for (int k=0; k<numNodes; k++) {
					for (int j=0; j<numNodes-k-1; j++){
						if(backoffNodes[j]>backoffNodes[j+1]){
							int swap = backoffNodes[j];
							backoffNodes[j] = backoffNodes[j+1];
							backoffNodes[j+1] = swap;
						}
					}
				}
				// check sort
				for (int i=0; i<numNodes; i++) {
					printf(" \n check sort 2 %d " , backoffNodes[i]);
				}
				// transmit collided backoff	
				delayNode[i] = DIFS + backoffNodes[i] + (maxFrameSize*8/dataRate); // delay time for node i transmits successfully.
				frameDataCollision = delayNode[i];
				Global_Timer = Global_Timer + delayNode[i]; // update the total delay
				// being transmitting
				while (frameDataCollision!=0) {
					frameDataCollision = frameDataCollision -1;
				}	
				// note that frame is transmitted
				if (frameDataCollision ==0){
					printf("\n Collided Frame is transmitted!");
					
				} 
			}
			
	}
	printf("\n Number of Collision %d", numCollision);
	printf("\n Total delay %d", Global_Timer);
	
	while(1){};

	
} // end main()
