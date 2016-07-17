.. include:: replace.txt

.. highlight:: cpp

.. heading hierarchy:
      ------------- Chapter
   ************* Section (#.#)
   ============= Subsection (#.#.#)
   ############# Paragraph (no number)

Applications
------------

Class ns3::Application can be used as a base class for |ns3| applications.
Applications are associated with individual nodes.  Each node
holds a list of references (smart pointers) to its applications.

Conceptually, an application has zero or more ns3::Socket
objects associated with it, that are created using the Socket
creation API of the Kernel capability.  The Socket object
API is modeled after the
well-known BSD sockets interface, although it is somewhat
simplified for use with |ns3|.  Further, any socket call that
would normally "block" in normal sockets will return immediately
in |ns3|.  A set of "upcalls" are defined that will be called when
the previous blocking call would normally exit.  THis is documented
in more detail in ns3::Socket class in socket.h.

The main purpose of the base class application public API is to
provide a uniform way to start and stop applications.

This chapter mainly discusses the base class Application.  Individual
applications will typically have their own chapter, but many of these
are not written yet.

Furthermore, as of ns-3.24, some applications that are more like
IP 'daemons' (like radvd, the routing advertisement daemon) were
factored into a separate module called ``internet-apps``.

The following |ns3| applications exist:

* BulkSendApplication
* OnOffApplication
* PacketSink
* UdpClient and UdpServer
* UdpEchoClient and UdpEchoServer 
* UdpTraceClient

Model Description
*****************

The source code for applications base class lives in the directory
``src/applications``.  

Design
======

To be completed.

Usage
*****
To be completed.

Helpers
=======

To be completed.

Examples
========

To be completed.

Validation
**********

Validation of individual applications is covered in their respective
chapters.

