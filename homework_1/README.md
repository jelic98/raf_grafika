[![License](https://img.shields.io/badge/License-Apache%202.0-blue.svg)](https://opensource.org/licenses/Apache-2.0)

# Homework 1

Command line tool for batch photo editing. There are two runtime modes. In the first one (non-interactive), user loads a command file in which are specified list of images and effects to be applied on those images. Second mode (interactive) gives user control of which effects to accept or reject.

![Interactive mode](https://github.com/jelic98/raf_rg/blob/master/homework_1/demo/interactive.png)

## Installing

1. Clone repository

```bash
git clone https://github.com/jelic98/raf_rg.git
```

2. Move into newly created directory

```gradle
cd raf_rg/homework_1
```

## Usage

1. Create new command file

```bash
touch test.cmd
```

2. Load batch of images and specify output path

```bash
echo "LOAD demo/test1.png output1.png" >> test.cmd
echo "LOAD demo/test2.png output2.png" >> test.cmd
```

3. Specify sequence of effects to be applied on each image

```bash
echo "RECT 200 200 150 50 00FF00FF FF0000FF" >> test.cmd
```

4. Grant execution privilege to run script

```bash
chmod +x run.sh
```

5a. Run tool in non-interactive mode

```bash
./run.sh test.cmd
```

5b. Run tool in interactive mode

```bash
./run.sh -i test.cmd
```

## Manual

For advanced usage, take a look at this awesome [manual](https://github.com/jelic98/raf_rg/blob/master/homework_1/MANUAL.md).

## Contributing

Pull requests are welcome. For major changes, please open an issue first to discuss what you would like to change.
