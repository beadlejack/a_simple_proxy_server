# Accidental Proxy Server 

I made this by accident. For my distributed systems course, I was supposed 
to create a webserver for the university, but misread the assignment. So 
surprise, here's a super simple proxy server that's basically just a middleman
between you and the actual site you're looking for. It does error checking 
for malformed HTTP requests. 

Written in C, uses basic socket programming. 

Usage: ./proxy -document_root <path_to_folder> -port <portno>

The document_root parameter is a leftover artifact from the actual assignment. 
For more info on the webserver assignment, the code is in [this repo](https://github.com/beadlejack/scu_webserver)
