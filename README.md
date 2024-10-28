It is a command line tool to filter (text) lines from shell pipline.
Usually it can be used with the 'cat' command to search the contents of text files:
cat xxx.log |kwlf "error" # To find the line containing 'error' in the log file

### Multiple keyword searches:
```
echo "hello, world, welcome to github! Today is 2024-10-01. The weather is sunny." |./kwlf "welcome" github
```

The output should be: "hello, world, welcome to github! Today is 2024-10-01. The weather is sunny.", words 'welcome' and 'github' will be highlighted.

### Regular expression matching
```
echo "hello, world, welcome to github! Today is 2024-10-01. The weather is sunny." |./kwlf  -r."[0-9]{4}-[0-9]{2}-[0-9]{2}"
```
The output should be: "hello, world, welcome to github! Today is 2024-10-01. The weather is sunny.". The date part should be highlighted.

### Exclusion Filtering
```
# Filter out lines that do not contain the specified word
echo "hello world!" |./kwlf -n."world"
```
This line should not appear in output. Becasue the word 'world' appears in the sentence.

### Completely matches all rules
By default, if any of the provided rules are met, they will be added to the output.
We can also require that all rules are met before adding to the output:

```
echo "hello world!" |kwlf -a hello github # This line will not be send to output
echo "hello world!" |kwlf -a hello world # This line will be send to output.
```
