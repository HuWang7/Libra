# Libra
“Libra” is a two-dimensional, top-down tank shooter game with twin-stick controls, created using my C++ game engine. The game features 3 distinct levels. Players must navigate these levels, tactically evading and eliminating four types of AI enemies, each equipped with their own unique abilities.

"E" controls the Tank's advance.
"S" controls the Tank's to turn left.
"F" controls the Tank's to turn right.
"D" controls the Tank's back
"SPACE" shoots bullets.
"M" Fire Weapon
"I,J,K,L" Control the rotation of the turret


"N" respawns after death.
"O" to run the game for the next frame and pause it.
"P" controls the pause and resume of the game.
"T" is held down to make the game run at 1/10th of the speed.
"H" Show HP Bar or disable HP Bar
"ESC" to exit the game (pause in game mode).
"F1" Game Debug Drawing
"F2" Tank can't be hit by bullet.
"F4" Open full map view/ tank view
"F6" Open/change Heat map mode
"F8" To restart the game.
"F9" Next Level

"~" Open/Close the console.
"A" Test console's AddLine()
"B" Test commands with args
"V" Unsubscribe from commands with no args
"C" Test console's Execute()

Reflecting on the development process of the Libra game, it's clear that tackling a complex programming project like this involves a steep learning curve and significant challenges, especially when it comes to understanding and implementing intricate systems and mechanics.

Initially, the programming tasks seemed daunting. One of the early stumbling blocks was understanding the keystroke input system. In previous assignments, keystroke handling was straightforward. However, in Libra, the approach to capturing keystroke inputs was different, and it involved working with arrays in a way that wasn't immediately intuitive. It took several hours of trial and error, and a bit of insight from a friend, to realize that the arrays related to keystrokes weren't being reset with each input. Instead, they were designed to update specific variables based on the keystroke entered. This revelation was a turning point in the project.

Once this concept was grasped, the subsequent programming tasks became more manageable. There were minor glitches along the way, but they were not insurmountable. The utilization of video resources played a crucial role in overcoming these challenges. Without such aids, the journey might have been even more arduous.

A fundamental issue encountered during the development was the lack of a strong programming foundation. This, coupled with a need to understand the architecture and interconnectedness of various components of the game, added to the initial difficulty. For instance, in Game.cpp, the interactions between different entities and some game mechanics were outlined. Understanding how different parts of the game, such as entities, the app, and their interrelations, work together was crucial.

After grasping the structure of the game and the function of each component, writing the code for the Libra assignment became more fluid. Recognizing the importance of how different programs connect and interact within the game was a key learning point. It not only made the coding process smoother but also provided a deeper understanding of game development as a whole.

In retrospect, working on Libra was both challenging and enlightening. It highlighted the importance of a solid programming foundation, a clear understanding of the system architecture, and the value of external learning resources. This experience has not only improved coding skills but also provided valuable insights into the complexities of game development.
