![Imgur](https://i.imgur.com/2Iqka11.png)

Way back in 2014, I learnt my first programming language: JavaScript. 
I spent most of the year 2015 working on my first major project: a pixel-art drawing tool. 
By November-ish, I had a working tool that I used for drawing game sprites in future years
(see my Github profile for a handful of games built using this tool).
However, this tool is starting to show its age. Some everyday operations are quite tedious.
Saving the animation as a sequence of PNGs requires navigating a menu and exporting
each frame one-by-one. That's just one example.

I have decided to redesign and rewrite this application in C++ using the Qt framework.
My experience using the old tool for drawing sprites and performing the tedious operations
should influence the design of this application. I aim to build something that I'll be able
to use for future projects. I'll certainly use this tool if no-one else does!

Most of the business logic of the application is complete. The majority of the time spent on
this project is building the UI with Qt. Below is a screenshot of what I have so far.

![Screenshot](https://i.imgur.com/GLgM21e.png)

The tools are fully implemented but you can't change the colors (no color picker) 
and you can't switch layers or frames (no timeline).
I plan on putting a timeline down the bottom and a color/palette menu on the right.
There's still a lot to do and given the free time I have nowadays (far less than I had in 2015),
this may end up taking a year as well!
