# Reference for implemented game modes
## Pentomino C (revision 1.3)
Pentomino C is an original game mode which introduces the 18 pentomino pieces in addition to the 7 tetrominoes, for a total of 25 pieces. It uses a complex and delicately balanced randomizer function to ensure that pieces are dealt to the player with a distribution that is predictable and gentle enough to feel fair.
### Levels
Pentomino C ends at level 1200. At level 1000 there is a timer-stop: if the player reaches level 1000 with greater than 10 minutes on the timer, the game abruptly ends. From level 1000 onward, the game speeds up, and begins dealing out significantly less friendly piece distributions, which become exponentially more difficult until level 1160 where the difficulty maxes out until the end of the game.
### Grade System
The grading in Pentomino C is entirely score-based, and uses the same score values as the G1 modes to assign grades. At levels 500 and 1000, there are score and time requirements which, if passed, grant the player access to the grades of M (Master) and GM (Grand Master).
### Requirements for M and GM
- At level 500: Score >= 50,000 and time <= 5:30
- At level 1000: Score >= 126,000 and time <= 10:00 (M awarded at level 1000)
- At level 1200: Survive (GM awarded at level 1200)