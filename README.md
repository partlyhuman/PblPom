PblPom - a[nother] productivity timer for the Pebble watch
==========================================================

![Screencap](http://f.cl.ly/items/1d3y452F1j1F2i0L3P0x/capture-fast.gif "Simulated image - looks different on phone")

This is a customizable, multilingual, simple, and attractive work timer. You
can use it with the Pomodoro Technique® or other time management systems.
The use of vibration means you can use your timers without worrying about
disturbing others, or having it drowned out by your music.

Current languages include English and Spanish. 
We are looking for translators. Adding a language is easy. Fork the repository,
add your language to the `PomLanguage` enum in `pom_text.h`, and add an entry in
your language for every text key in `pom_text.c`. Submit a pull request.

This project uses the [httpebble library](https://github.com/Katharine/httpebble-watch) by Katharine Berry.
Your settings will be maintained as long as you have httpebble running in the background on your phone.
Phone and httpebble are both completely optional.

The "Pomodoro Technique®" and "Pomodoro®" are registered trademarks of 
Francesco Cirillo. We have no relationship with, nor do we endorse, these
marks, their owner, or the company FC Garage.
