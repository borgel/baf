# baf
*BAF* is a Basic Anmation Framrwork. You could even call it a crummy special purpose schedular of sorts. From a list of "animations" it queues "channel" changes at the interval specified, based on the progression of time.

BAF was written as the upper half of *YABI* [1], which knows how to consume these channel changes and execute them on hardware.

Remember, perfect is the enemy of good enough.

### Future Features
* Support for other animation types (oneshot, looped, random loop)
* Support for starting an animation at the end of this one (partially complete)
* Fixes for rollovers, etc (see TODOs and FIXMEs)
* More I'm forgetting


[1]: https://github.com/borgel/yabi/
