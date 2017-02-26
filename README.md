#802-11-DCF-MAC implementation
        Author: Tung Thanh Le
        Contact: ttungl at gmail dot com
###General description:
In this course project, we are asked to implement the RTS/CTS mode into a wireless network medium. 
A network topology is created as well as source destination pairs are involved. In each 2x2, 4x4, 8x8,
10x10 networks, we assume that the even nodes are the source nodes, and the odd nodes are
the destination nodes. Particularly, in 3x3 network, the number of nodes is an odd number, 
we force the last node to become a source node. For example, in 3x3 network, we have 9 nodes,
including 4 pairs and 1 source node will be connected to the adjacent node (node 8 connects to
node 7). 

In terms of the collision domain, this project has non-uniform collision domain, compared with the
previous work (uniform collision domain). For example, each node has its transmission range, 
so if other nodes that reside inside this footprint would probably impact to that node’s transmission, 
especially in the case of the same backoff timers. If other nodes are outside the range of 
the node’s transmission, they can transmit without any collision.

If a node fails to transmit CTS or ACK due to the bad channel conditions, it retries around 7-
8 times before dropping that frame. 
