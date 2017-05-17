# Borderless Window Helper

## Description

Borderless Window Helper is a tool that primarily minimizes the borderless windows of games when they are not in the foreground, and as a secondary function, it can also "maximize" and remove the borders of game windows that are not full-screen. It works by maintaining a list of processes whose windows you want to affect, monitoring the windows of those processes when they run.

## How to use

Download the latest version (linked below), then unpack the archive into a new folder at a location of your choice, and run `Borderless Window Helper.exe` (there's no installation, the program is portable).

The interface is pretty straight-forward -- the list on the right shows you relevant processes that are currently running, and the list on the left shows you the processes (presumably games) whose windows the program currently affects. You must identify the executable of a game you want to affect in the right list, and double-click it to add it to the left list. Once that's done, the program will automatically minimize that game's window when you alt-tab out of it (or whenever you perform any action that removes that window from the foreground, such as pressing Win+D or Win+M). If the window in question is not borderless, the program will remove its borders and resize it to fill the screen (in a monitor-aware fashion).

Pressing the "close" button only hides the interface, so if you want to close the program, right-click on the tray icon and select "Exit". And if you're going to use the program regularly, you'll probably want to run it automatically at system startup, which you can do by selecting "Start with Windows" from the tray menu. FYI, that results in a .lnk file being placed in `%appdata%\microsoft\windows\start menu\programs\startup` (you can use `shell:startup` as a short alias for that path).

## Download


[Borderless Window Helper 1.0.zip](https://github.com/ErrorFlynn/Borderless-Window-Helper/releases/download/v1.0/Borderless.Window.Helper.1.0.zip)

![bwh-1](https://cloud.githubusercontent.com/assets/20293505/26132606/6f7c6fa8-3a6f-11e7-9b08-44cd5b8eb4e7.png)
