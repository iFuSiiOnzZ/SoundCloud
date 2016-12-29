# SoundCloud
A small application to download songs from SoundCloud

## Downlod link
[OneDrive](https://1drv.ms/u/s!AirMY7s72T8BvF7Sh6DnctdMwqwL)

## Usage

The application can be executed in console mode or using the user graphic interface. For user interface just click the exe, for console open a command line a type SoundCloud.exe -s song_url.

![alt tag](http://i.imgur.com/FFmH4nP.png)

```
SoundCloud.exe -s song_url [-p save_path]
    > -s soundcloud song url
    > -p path to save the mp3

    NOTE: Parameters inside [...] are optional
    EXAMPLE: SoundCloud.exe -s "https://soundcloud.com/thefatrat/thefatrat-monody-feat-laura-brehm-1" -p "D:\Music\TheFatRat"
```

## Remarks
* Tested in Window 10 x64 under Visual C++ Build Tools 2015 - standalone C++ tools
* If for any reason track can't be saved with its name it will be saved with a timestamp
