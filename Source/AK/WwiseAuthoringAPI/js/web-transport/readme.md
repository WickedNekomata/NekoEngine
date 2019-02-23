# Web transport sample

## Overview 

The web transport sample allows to launch transport commands from a web page. It connects to the currently opened project and makes the project's objects available to query through the combobox. When an object is selected, it creates a transport for that object and makes available the transport commands such as play, stop and pause. 

It demonstrates the following functionalities:
* Retrieving objects from a project using a search query
* Execute transport commands
* Update on transport events (buttons)

## Prerequisites

Supported browsers are Google Chrome, Mozilla Firefox and Opera.

## Installation

Install `git`. Download fom <https://git-scm.com>.

Install `node.js`. Download fom <https://nodejs.org>.

Install `bower` globally:

    npm install -g bower

Install the dependencies with `bower`:

    bower install

## Execution

Double-click on `index.html`