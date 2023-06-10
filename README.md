# P2P File Searching System - Chord DHT Protocol

<b> Author:  </b> Vikram Bala <br>
<b>Email: </b> vikrambala2002@gmail.com <br>
<b>Code Repository: </b> [GitHub Link](https://github.com/vbala29/Chord-P2P-File-Sharing) <br>
<b> YouTube Demo: </b> <br>

<b> What this is: </b> This project is an application (intended to run on Ubuntu or MacOS Ventura 13.1+) that implements the [Chord Distributed Hash Table Protocol](https://pdos.csail.mit.edu/papers/ton:chord/paper-ton.pdf), one of the four original DHT protocols, which won an ACM SIGCOMM award in 2011. More specifically, the application uses the Chord protocol to provide users with a P2P Document Search-Engine with which they can simply type in a search keyword, and receive a list of all available documents matching that keyword; the Chord protocol provides (with high probability) that such information is retrieved with O(log N) nodes contacted in an N node network. 

<b> Inspiration: </b> The ideas in this project were originally implemented as the final project for the CIS5530 Networked Systems Class at the University of Pennsylvania (my partners including Ryan Oliver, Helen Rudoler, and Cassandra Horwege), but were only implemented in the [NS-3 network simulator](https://www.nsnam.org). However, I wanted to take this project and create it in the real world, where I could demonstrate its functionality on actual host machines (see YouTube video for demonstration). More specifically, I was interested in gaining a deeper understanding of the Linux networking stack at the OS level, as well as the Linux pthread library, features which I were not able to use when working with the NS-3 simulator. Implementing the Chord protocol and an associated file search system as an application helped me gain valuable experience in debugging multithreaded, distributed applications, as well as further developing my knowledge of distributed hash table algorithms, networking protocols, serialization, and how to improve the performance of networked systems. The nature of this application is reminiscent of many P2P file sharing systems of the past such as (Napster)[https://en.wikipedia.org/wiki/Napster] and (Gnutella)[https://en.wikipedia.org/wiki/Gnutella], and those served as inspiration for this project. 

<b>Technology:</b> This aplication was origina
A P2P (Peer to Peer) document sharing service that implements the Chord distributed hash table protocol. Clients run a multithreaded application (enabled by a custom thread pool library) to find and retrieve files with algorithmically gauranteed efficiency.

Chord Papers: https://pdos.csail.mit.edu/papers/ton:chord/paper-ton.pdf, https://pdos.csail.mit.edu/papers/chord:sigcomm01/chord_sigcomm.pdf
