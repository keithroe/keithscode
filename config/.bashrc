
# 
#  author : R. Keith Morley
#  filename : .bashrc
#  
#  very simple .bashrc.  if you happen to see this and have any suggestions
#  let me know (rmorley@cs.princeton.edu)
#


# Define some colors first.  CAPS are bold 
red='\e[0;31m'
RED='\e[1;31m'
blue='\e[0;34m'
BLUE='\e[1;34m'
cyan='\e[0;36m'
CYAN='\e[1;36m'
NC='\e[0m'            


# find what OS we are running 
case $OSTYPE in
    *[Ll]inux*)
    OS="linux"
    ;;
    *[Dd]arwin*)
    OS="darwin"
    ;;
    *[Ii][Rr][Ii][Xx]*)
    OS="irix"
    ;;
    *)
    echo ".bashrc: Unknown OS type \'$OSTYPE\'"
    ;;
esac

#
#  These should be constant across different OSes
#
export CVSROOT=:ext:keithroe@cvs.sourceforge.net:/cvsroot/galileorenderer
export CVS_RSH=ssh
export SVN_EDITOR=vim
export GR_PATH=../galileo
export GR_TEXTURE_PATH=../
export RUBYLIB=~/software/myruby
export TERM=xterm-color
export PS1="\[$blue\][\u] \W $ \[\033]0;[\u@\h] \w\007$NC\]"
#export PS1="\[$blue\][\h] \W $ \[\033]0;[\u@\h] \w\007$NC\]"
export PATH=$PATH:~/bin/
export PATH=/usr/local/cuda/bin:$PATH
export PATH=/Code/p4r/p4review/:$PATH
export DYLD_LIBRARY_PATH=/usr/local/cuda/lib 
export P4CONFIG=.p4config
#export SCENIXHOME=/home/kmorley/Code/scenix/sw/wsapps/nvsgsdk/nvsgmain/
#export NVSGSDKHOME=/home/kmorley/Code/scenix/sw/wsapps/nvsgsdk/nvsgmain/
#export BUILD_TOOLS_DIR=/home/kmorley/Code/scenix/sw/tools
export GEM_PATH=/home/kmorley/.gem
export OPENIMAGEIOHOME=/usr/local/oiio/
export PERL5LIB=~/Code/rtsdk/gtl_test/submission_script/extlib/

#function myp4review
#{
#    review=$1
#    dir=/tmp/review/$review
#    rm -Rf $dir
#    p4mount
#    mkdir -p $dir
#    pushd $dir
#    unzip /mnt/netapp-hq/p4review/cl-$review.p4r
#    opendiff old new
#    popd
#}

function makep4review
{
    cl=$1
    p4review.pl -server -s $cl -p4client kws_rtsdk_lt -p4port p4proxy-slc:2006
}

function diffp4review
{
    cl=$1
    mount_p4
    p4review.pl /mnt/netapp-hq/p4review/cl-$cl.p4r -diff fmdiff
}

function myp4rmerge
{
    cl=$1
    ~/Code/sw/p4review/p4rmerge.pl -server $cl 
}


#
#  Now a few OS specific things
#
if [[ $OS == linux ]]; then
    # set any linux specific attrs
    export PATH=$PATH
else 
    if [[ $OS == irix ]]; then
    # set any irix specific attrs
    export PATH=$PATH
    else 
        if [[ $OS == darwin ]]; then
            # set any darwin specific attrs
            export PATH=/usr/local/bin:/opt/local/bin:$PATH
            test -r /sw/bin/init.sh && . /sw/bin/init.sh
        else
            echo "Failed to set OS specifics ... "
        fi
    fi
fi


source ~/.aliases
source ~/.gtl_signature

