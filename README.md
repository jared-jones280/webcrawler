# webcrawler
a windows (winsock) webcrawler

Part 1:
Using Visual C++, your goal is to create a simple web client that accepts URLs and then crawls
them to display basic server/page statistics. 

Part 2:
You will now expand into downloading multiple URLs from an input file using a single thread.
To ensure politeness, you will hit only unique IPs and check the existence of robots.txt. Robot
exclusion is an informal standard that allows webmasters to specify which directories/files on the
server are prohibited from download by non-human agents. See http://www.robotstxt.org/ and
https://en.wikipedia.org/wiki/Robots_exclusion_standard for more details. To avoid hanging the
code on slow downloads, you will also have to abort all pages that take longer than 10 seconds1
or occupy more than 2 MB (for robots, this limit is 16 KB).

Part 3:
We are finally ready to multi-thread this program and achieve significantly faster download
rates. Due to the high volume of outbound connections, your home ISP (e.g., Suddenlink, campus dorms) will probably block this traffic and/or take your Internet link down. Do not be
alarmed, this condition is usually temporary, but it should remind you to run the experiments
over VPN. The program may also generate high rates of DNS queries against your local server,
which may be construed as malicious. In such cases, it is advisable to run your own version of
BIND on localhost. 
