德州AI，德州扑克AI，出售AI源码和训练模型；联系Telegram：@xuzongbin001或E-mail：taibaibaba1314@gmail.com
# MasterAI-1.0-1vs1-Limit

## Introduction

MasterAI is an AI poker dedicated to suport n-play (single- or multi-agent) Texas Hold'em imperfect-informatin games. It has been achieving exceptionally good results by using its propretary algorithm. In September 2020, it defeated 14 top human poker professionals. 
MasterAI 是Master团队在非完美信息博弈中实现的的一种扑克AI，在德州扑克一对一的有限押注已经取得一定成果，MasterAI于2020年9月战胜了中国的14位顶级扑克职业选手。

## Technology

MasterAI is developed by two components: offline component and online component:
* An offline component solves random poker situations (public states along with probability vectors over private hands for both players) and uses them to train a neural network. After training, this neural network can accurately predict the value for each player of holding each possible hand at a given poker situation.

* An online component uses the continuous re-solving algorithm to dynamically select an action for MasterAI to play at each public state during the game. This algorithm solves a depth-limited lookahead using the neural network to estimate values at terminal states.

The MasterAI strategy approximates a Nash Equilibrium, with an approximation error that depends on the neural network error and the solution error of the solver used in continuous re-solving.

## Prerequisites

Running any of the MasterAI code requires [torch](http://torch.ch/).
Torch is only officially supported based on *Linux systems. [This page](https://github.com/torch/torch7/wiki/Windows)
contains suggestions for using torch on a Windows machine

If you would like to personally play against MasterAI, We provide AI models to challeng,[click here](https://master.deeptexas.ai/aigame/).

## Contact us

The Master team is constantly exploring the innovation of AI algorithm, and hoping that like-minded technical experts from all over the world can communicate and exchange here, or join us to make MasterAI bigger and stronger together. Please feel free to contact us at Telegram：@alibaba401


MasterAI，于2020年9月 战胜14位中国顶级职业德扑高手

MasterAI 是Master团队在非完美信息博弈中实现的的一种扑克AI，在德州扑克一对一的有限押注（0~100BB）已经取得一定成果，MasterAI于2020年9月战胜了中国的14位顶级扑克职业选手；在与国内14位顶尖牌手激烈角逐31561手牌后，MasterAI 最终以23,562总计分牌，每百手赢取36.38个大盲的优势取胜。MasterAI 基于深度学习，强化学习和博弈论，采用Nash纳什均衡的对战策略，通过大量MC蒙特卡洛采样来计算CFR (虚拟遗憾最小化)的值域或频域作为行动Value，不断探索和选取GTO最优策略，形成智能分析和决策。
        
MasterAI赛事情况如下 ：
![640](https://github.com/user-attachments/assets/8982ce0a-4d9b-4c55-bfb2-ec8228e1a23a)
![640 (1)](https://github.com/user-attachments/assets/4c5591c7-e59a-4fde-8af9-723243ce0cf1)


 9/1~9/4 首届全明星邀请赛，MasterAI 机器人已战胜顶尖扑克游戏职业高手每百手赢取大盲达到平均36.38的水准，大赢人类职业选手。

![微信图片_20241030105723](https://github.com/user-attachments/assets/3d473e19-db23-4cf2-a4d2-50d73cb8ab77)
![微信图片_20241030103520](https://github.com/user-attachments/assets/3fd8c2d9-8dde-42a9-a82f-1f8677610735)


有对Master AI  1对1 训练模式和核心算法代码感兴趣或者意向购买源码者可以联系：Telegram：@xuzongbin001



        
