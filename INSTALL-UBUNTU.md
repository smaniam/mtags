sudo apt-get install \
  build-essential \
  subversion \
  mercurial \
  git-core \
  cmake \
  libfreetype6 \
  libfreetype6-dev \
  libtool


wget http://poppler.freedesktop.org/poppler-0.16.2.tar.gz
tar xf poppler*
rm poppler*.gz
cd poppler*/
./configure
make
sudo make install
sudo mkdir -p /usr/local/include/poppler/goo
sudo cp -a poppler/*.h /usr/local/include/poppler/
sudo cp -a goo/*.h /usr/local/include/poppler/goo/

cp exiv2-0.21.1/src/exiv2.hpp /usr/local/include/exiv2/
wget http://www.exiv2.org/exiv2-0.21.1.tar.gz
tar exiv2*
cd exiv2*
#svn checkout svn://dev.exiv2.org/svn/trunk


