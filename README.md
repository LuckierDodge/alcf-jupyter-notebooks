# Jupyter Notebooks and Reference Material

## Setup

1. Install Miniconda
1. Run `ssh -N -f -L 8888:localhost:9000 luckierdodge@flick.cs.niu.edu` in the root of this repository.
1. Setup Jupyter server password access
1. Run `jupyter lab` on the server, preferably in a tmux or screen session one can detach from.
1. Run `ssh -N -f -L 8888:localhost:9000 <user>@<hostname>` on a local machine you want to connect to your jupyter notebook through, replacing the first port number with the port you want to connect to locally, and the second port number with the port you have setup your jupyter notebook to use on the remote server.
