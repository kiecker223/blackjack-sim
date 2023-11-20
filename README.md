## BLACKJACK SIM

Currently under development. Focus on more intuitive scriptability than other competetors, mostly made for myself though because other tools were too complicated or didn't have the features I wanted.


#### Design Philosophy

Focus on speed by minimizing allocations, trying to keep everything cache-coherent, and opting for switch statements rather than if-statements. So far the only bottleneck that I have currently measured is a single cache miss by
```cpp
_dh.Reset();
```
in BjGame.cpp. Hopefully speed improvements in the future. Currently single threaded in my VM I can get 10k shoes played a second so that's not bad at all.