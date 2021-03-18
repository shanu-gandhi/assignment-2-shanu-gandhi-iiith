MiniTorrent <br>
(Explanation of my implementation)<br>
Assume<br>
1. We have received information (IP:Port) of two seeds/leeches for a particular file we<br>
want to download (from the tracker server).<br>
2. Want to download a file called “movie.mp4” with size 722mb<br>
Piece = individual data unit. Analogous to “page” concept in OS.<br>
Piece size fixed arbitrarily (Ex – 128kb)<br>
How concurrent download happens<br>
  2.1. Create a file named “movie.mp4” and write 0s in the file until the file size is 722mb.<br>
(A file with appropriate size is created with dummy data)<br>
  2.2. Spawn two threads (one for each seed/leech), first thread is responsible for<br>
downloading the first half pieces of the file (ex – first 50 out of 100 pieces), 2nd<br>
thread is responsible for downloading second half pieces of the file. (Equal<br>
assignment initially)<br>
3. Piece download works like this – A thread will request a particular piece number<br>
from a peer, if that piece number is available at that peer, it will send piece data.<br>
Otherwise it will return an opcode which says “Piece not available”.<br>
4. A mutex lock is used before accessing the file. Each thread, after download a piece<br>
from a peer, will lock the mutex, offset to the appropriate distance in the file<br>
(depending on the piece number) and write the download piece data and then<br>
unlock the mutex.<br>
5. After trying to download its assigned pieces, each thread will check which pieces are<br>
left to be downloaded, and then request those pieces from their peer. Therefore, if a<br>
piece is available at atleast one peer, it will be downloaded.<br>
Several optimizations which can be done:<br>
1. Piece selection algorithm used here is random/uniform. Prior information can be<br>
obtained from each peer as to who has which pieces, piece requests can be<br>
optimized<br>
2. Some peers may have more processing power or bandwidth compared to other<br>
peers. Can design a system such that those peers receive more piece requests.
