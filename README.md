# Simulator for ARC-LEACH

## Setup

### Using Provided NS3 Instance
- Check for [dependencies](https://www.nsnam.org/wiki/Installation#Fedora.2FRedHat)
- Good to go

### Using Own NS3 Instance
- Go into config dir
- Copy contents of contrib to ns3 contrib folder
- Copy contents of scratch to ns3 scratch folder
- Run ./ns3 configure again
- Good to go 

## Config

### scratch/{protocol}.cc
- Number of nodes
- Number of malicious nodes

### contrib/leach/model{protocol}.cc
- Number of rounds
- Number of events
- Probaility of cluster size
