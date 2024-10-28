mkdir build

cd build

cmake ..

make

echo "filter welcome & github"
echo "hello, world, welcome to github! Today is 2024-10-01. The weather is sunny. >>>" 
echo "hello, world, welcome to github! Today is 2024-10-01. The weather is sunny." |./kwlf "welcome" github 

echo "filter date (xxxx-xx-xx)"
echo "hello, world, welcome to github! Today is 2024-10-01. The weather is sunny. >>>"
echo "hello, world, welcome to github! Today is 2024-10-01. The weather is sunny." |./kwlf  -r."[0-9]{4}-[0-9]{2}-[0-9]{2}"

echo "filter line do not contains 'world'."
echo "hello, world! >>>"
echo "hello, world!" |./kwlf -n."world"
echo "welcome to github! >>>"
echo "welcome to github!" |./kwlf -n."world"