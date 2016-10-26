![Kodi logo](https://raw.githubusercontent.com/xbmc/xbmc-forum/master/xbmc/images/logo-sbs-black.png)
**Welcome to Kodi's contributing and development guide!**

1. [Introduction](#introduction)  
  1.1 [Got questions?](#11-questions)  
  1.2 [Bug reports](#12-bug-reports)  
  1.3 [Git-Fu](#git-fu)  
2. [Getting started](#2-getting-started)  
3. [Git-Fu](#git-fu)  
3. [Code guidelines and formatting conventions](#code-guidelines-and-formatting-conventions)  

## 1. Introduction

Welcome to Kodi's contribuing and development guide. Information herein contained is purposely concise, giving you an introduction to Kodi's development process, tools used and workflow.

**Kodi** uses Github for development only, i.e. for *pull requests* and the review of such code.  
**Do not open** an issue on Github for your questions or bug reports.  
**Do not comment** on a *pull request* unless you are involved in the testing of such or have something meaningful to contribute.  

### 1.1 Got questions?

To get your questions answered, please ask in the [Kodi community forum's](http://forum.kodi.tv/) in the appropriate section or on **IRC** [#kodi](http://webchat.freenode.net?nick=kodi-contrib&channels=%23kodi&prompt=1&uio=OT10cnVlde) channel on freenode.net

**TODO:** change channel to #kodi-dev

### 1.2 Bug reports

Issue or bug reports are created and reviewed at [Kodi's bug tracker](http://trac.kodi.tv) using the Kodi forum's *username* and *password*.
If you can, we encourage you to investigate the issue yourself and create a [pull request](https://help.github.com/articles/creating-a-pull-request/) for us to review.


## 2. Getting started

If you don't have a GitHub account, now is the time to [sign-up](https://github.com/join/).  
Next, you need your own copy of Kodi's source code to work. In GitHub lingo, these are
- [Click here](https://github.com/xbmc/xbmc/fork/) to **fork Kodi** project,
   and [configure the remote](https://help.github.com/articles/configuring-a-remote-for-a-fork/):

   ```bash
   # Clone your fork of kodi's repo into the current directory in terminal
   git clone git@github.com:<your github username>/xbmc.git kodi
   # Navigate to the newly cloned directory
   cd kodi
   # Assign the original repo to a remote called "upstream"
   git remote add upstream https://github.com/xbmc/xbmc.git
   ```




For bug reports and related discussions, feature requests and all other support, please go to [Kodi community forum's](http://forum.kodi.tv/).


Not familiar with git? Start by looking at Github's [collaborating pages](https://help.github.com/categories/collaborating/).





#### Pull Requests

Before [creating a pull request](https://help.github.com/articles/creating-a-pull-request/), please read our general code guidelines that can be found at:

- [Code guidelines](https://codedocs.xyz/xbmc/xbmc/code_guidelines.html)
- [Submitting a patch](http://kodi.wiki/view/HOW-TO_submit_a_patch)
- [Kodi development](http://kodi.wiki/view/Development)

##### General guidelines for creating pull requests:
**Create topic branches**. Don't ask us to pull from your master branch.  
**One pull request per feature**. If you want to do more than one thing, send multiple *pull requests*.  
**Send coherent history**. Make sure each individual commit in your *pull
  request* is meaningful.  
  If you had to make multiple intermediate commits while developing, please squash them before sending them to us.  
  In the end before merging you may be asked to squash your commit even some more.  

###### Please follow these guidelines; it's the best way to get your work included in the project!

- [Click here](https://github.com/xbmc/xbmc/fork/) to **fork Kodi** project,
   and [configure the remote](https://help.github.com/articles/configuring-a-remote-for-a-fork/):

   ```bash
   # Clone your fork of kodi's repo into the current directory in terminal
   git clone git@github.com:<your github username>/xbmc.git kodi
   # Navigate to the newly cloned directory
   cd kodi
   # Assign the original repo to a remote called "upstream"
   git remote add upstream https://github.com/xbmc/xbmc.git
   ```

- If you cloned a while ago, get the latest changes from upstream:

   ```bash
   # Fetch upstream changes
   git fetch upstream
   # Make sure you are on your 'master' branch
   git checkout master
   # Merge upstream changes
   git merge upstream/master
   ```

- Create a new topic branch to contain your feature, change, or fix:

   ```bash
   git checkout -b <topic-branch-name>
   ```

- Commit your changes in logical chunks, or your *pull request* is unlikely to be merged into the main project.  
   Use git's [interactive rebase](https://help.github.com/articles/interactive-rebase)
   feature to tidy up your commits before making them public.

- Push your topic branch up to your fork:

   ```bash
   git push origin <topic-branch-name>
   ```

-  Open a [pull request](https://help.github.com/articles/using-pull-requests) with a 
   clear title and description.

## Git-Fu

Insert text here

## Code guidelines and formatting conventions 

```c++
namespace KODI
{
namespace UTILS
{
class ILogger
{
  void Log(...) = 0;
}
}
}
```

```c++
#include "PVRManager.h"
#include <cassert>
#include <utility>
#include "Application.h"
#include "Util.h"
#include "addons/AddonInstaller.h"
#include "dialogs/GUIDialogExtendedProgressBar.h"
#include "messaging/helpers/DialogHelper.h"
#include "music/tags/MusicInfoTag.h"
#include "network/Network.h"
#include "pvr/addons/PVRClients.h"
#include "pvr/channels/PVRChannel.h"
#include "settings/Settings.h"
#include "threads/SingleLock.h"
#include "utils/JobManager.h"
#include "utils/log.h"
#include "utils/Variant.h"
#include "video/VideoDatabase.h"
```
