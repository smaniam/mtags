sudo apt-get install \
  cmake
  libfreetype6 \
  libfreetype6-dev \
  libtool

git clone git://anongit.freedesktop.org/fontconfig
./autogen.sh >/dev/null 2>/dev/null
./configure
make
sudo make install

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
