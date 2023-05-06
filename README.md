# Yu-Gi-Oh! Forbidden Memories Background Image Extractor

This is a utility designed to extract background images out of the game in DDS format.

Currently it only supports version SLUS-01411.

Currently it can only extract the basic background images found during the campaign mode.

## TODO list

- add support for images type 1 and 2 (0x1xx and 0x2xx)

- add version detection and support for other versions

- maybe look into extracting other images and content

- maybe add repacking support

## How to use

- Extract `WA_MRG.MRG` out of the disc image

- In the OS shell launch the app with following arguments: `YGOFM-BGEx WA_MRG.MRG BGIndex` where `BGIndex` is the number of the background you wish to extract

- If you wish to not untile the image, append 0 at the end, like this: `YGOFM-BGEx WA_MRG.MRG BGIndex 0`

## How does it work

### LBA calculation

The magic can be found within the `calclba` function.

The game stores its data in LBAs, meaning each data block is aligned to 0x800 bytes.

Backgrounds in WA_MRG.MRG start at LBA `0x21D5` for the US version of the game and may vary across regions.



These LBAs are hardcoded during compile time in the game executable, so it will differ per each region/variant of the game.

The function responsible for calculating LBAs for these background images can be found at offset `0x8002DF2C` of the game executable.



Keep in mind that due to the odd way of calculating LBAs there are repeated images despite having different indicies.

### Image conversion

These images are stored in 256 color palletized format.

PS1 pallette is in RGB555 format, with the 15th bit being the "special transparency bit".

The tool converts them to a raw DDS ARGB image without any data loss.


