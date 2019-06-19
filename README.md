![Imgur](https://i.imgur.com/2Iqka11.png)

## A picture is a thousand words (scroll down for GIFs!)

![Screenshot](https://i.imgur.com/q9RILMu.png)

## A story...

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

## Progress

Most of the business logic of the application is complete. The majority of the time spent on
this project is building the UI with Qt.

The tools are fully implemented but there's no UI to configure them (e.g. change the brush size 
or symmetry mode) and you can't switch layers or frames (no timeline).
I plan on putting a timeline in the big rectangle down the bottom and the tool settings will
go on the right side of the status bar (under the timeline).

## Demos

Here's a few quick gifs that demostrate some of the features I have implemented so far.

![Magic Wand](https://i.imgur.com/kmQziuq.gif)

![Brush](https://i.imgur.com/awJ8cOf.gif)

![Fill](https://i.imgur.com/15cNBDr.gif)

![Rect Select](https://i.imgur.com/JdbRhW8.gif)

![Poly Select](https://i.imgur.com/KM8Sown.gif)

![Filled Circle](https://i.imgur.com/wVTqfYF.gif)

![Stroked Circle](https://i.imgur.com/8ntlkEW.gif)

![Filled Rect](https://i.imgur.com/lUGYKW2.gif)

![Stroked Rect](https://i.imgur.com/r7M8XUi.gif)

![Line](https://i.imgur.com/BQ6LEtj.gif)

![Translate](https://i.imgur.com/VlO5mrl.gif)

![Flip](https://i.imgur.com/HJnQuiK.gif)

![Rotate](https://i.imgur.com/Xj1AG30.gif)

![Color Picker](https://i.imgur.com/UCoMrYK.gif)
