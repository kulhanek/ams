# Standard NCBR .bashrc

# change default umask of the user
export INF_UMASK=022

# use system wide setup
if [ -f /etc/bashrc ]; then
    . /etc/bashrc
fi


