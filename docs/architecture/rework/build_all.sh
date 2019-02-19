

rm *.png

for i in *.mermaid; do

    fn=`basename $i .mermaid`
    echo "Building $fn.png"
    mmdc -i $i -o $fn.png
done
