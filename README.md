# gstdenoise
Gstreamer wrapper around https://github.com/lucianodato/noise-repellent

## Capabilities
- Autodetect noise and remove in live/non-live pipelines
- Send pure noise to the element and have it create a noise profile
- Load and apply noise profile in live pipeline
- Add a small amount of white noise to make the result sound more natural
- Listen to only the noise that would be removed (useful for making sure no signal is being removed)

##TODO
- Add compile instructions
- Add better documentation
- Clean up element settings to be more clear
