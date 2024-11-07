It is a command line tool to filter (text) lines from shell pipline.
Usually it can be used with the 'cat' command to search the contents of text files:

eg. Use 'cat xxx.log |kwlf error' To find the line containing 'error' in the log file

### Multiple keyword searches:
```
echo "hello, world, welcome to github! Today is 2024-10-01. The weather is sunny." |./kwlf "welcome" github
```
The output should be: "hello, world, **welcome** to **github**! Today is 2024-10-01. The weather is sunny.", words 'welcome' and 'github' will be highlighted.

### Regular expression matching
If the keyword is a regular expression, we need to add the '-r.' prefix to indicate it:
```
echo "hello, world, welcome to github! Today is 2024-10-01. The weather is sunny." |./kwlf  -r."[0-9]{4}-[0-9]{2}-[0-9]{2}"
```
The output should be: "hello, world, welcome to github! Today is **2024-10-01**. The weather is sunny.". The date part will be highlighted.

### Exclusion Filtering
By default, a string containing a keyword or matching a regular expression is considered to satisfy one of the rules.
We can add the prefix '-n.' to indicate that the keyword is not contained to satisfy the rule:
```
# Filter out lines that do not contain the specified word
echo "hello world!" |./kwlf -n."world"
```
This line should not appear in output. Becasue the word 'world' appears in the sentence.
Similarly, if you want to indicate that a regular expression is not satisfied, you can do this: '-rn.'


### Completely matches all rules
By default, if any of the provided rules are met, they will be added to the output.
We can also require that all rules are met before adding to the output with parameter '-a':

```
echo "hello world!" |kwlf -a hello github # This line will not be send to output
echo "hello world!" |kwlf -a hello world # This line will be send to output.
```

![image](https://github.com/user-attachments/assets/e2b86c1d-8b10-465a-b372-865a41145dab)

