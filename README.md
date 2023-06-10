# P2P File Searching System - Chord DHT Protocol

<b> Author:  </b> Vikram Bala <br>
<b>Email: </b> vikrambala2002@gmail.com <br>
<b>Code Repository: </b> [GitHub Link](https://github.com/vbala29/Chord-P2P-File-Sharing) <br>
<b> YouTube Demo: </b> <br>

<b> What This Is: </b> This project is a application (intended to run on Ubuntu or MacOS Ventura 13.1+) that implements the [Chord Distributed Hash Table Protocol](https://pdos.csail.mit.edu/papers/ton:chord/paper-ton.pdf), one of the four original DHT protocols, which won an ACM SIGCOMM award in 2011. <b>The application uses the Chord protocol to: 1. Allow users to join a network of other host computers with which they can share files. 2. Provide users with a P2P Document Search-Engine with which they can simply type in a search keyword, and then receive a list of all available documents in the network which match that keyword.</b> The Chord protocol provides (with high probability) that such information is retrieved with O(log N) nodes contacted in an N node network. The nature of this application is reminiscent of many P2P file sharing systems of the past such as [Napster](https://en.wikipedia.org/wiki/Napster) and [Gnutella](https://en.wikipedia.org/wiki/Gnutella), and those served as inspiration for this project. Note that this project doesn't provide the implementation for the actual sending of files between host machines; however, this is trivial to implement given the features that this project provides.

<b> Inspiration: </b> The ideas in this project were originally implemented as my final project for the CIS5530 Networked Systems Class at the University of Pennsylvania (my partners including Ryan Oliver, Helen Rudoler, and Cassandra Horwege), but were only implemented in the [NS-3 network simulator](https://www.nsnam.org). However, I wanted to take this project and create it in the real world, where I could demonstrate its functionality on actual host machines (see YouTube video for demonstration). More specifically, I was interested in gaining a deeper understanding of the Linux networking stack at the OS level, as well as the POSIX pthread library, features which I were not able to use when working with the NS-3 simulator. Implementing the Chord protocol and an associated file search system as an application helped me gain valuable experience in debugging multithreaded, distributed applications, as well as further developing my knowledge of distributed hash table algorithms, networking protocols, serialization, and how to improve the performance of networked systems. 

<b>Technology:</b> The application was written in C++14, and uses the POSIX pthreads library, C Sockets library, and OpenSSL library.

Chord Papers: https://pdos.csail.mit.edu/papers/ton:chord/paper-ton.pdf, https://pdos.csail.mit.edu/papers/chord:sigcomm01/chord_sigcomm.pdf

![Chord_network](https://github.com/vbala29/Chord-P2P-File-Searching/assets/56012430/f02f7dc5-02bb-4e27-8f2a-35c1b899f9b8)
