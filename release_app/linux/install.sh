#!/bin/sh

app_name=robotGui
app_dir=`pwd`/pkg
app=$app_dir/$app_name

exe=$app
icon=${app_dir}/icon/icon.jpg
newline="\n"

output="${app_name}.desktop"

if [ -f "$output" ]; then
	rm "$output"
fi

echo "[Desktop Entry]" >> "$output"
echo "Type=Application" >> "$output"
echo "Exec=bash -i -c ${exe}" >> "$output"
echo "Name=${app_name}" >> "$output"
echo "GenericName=${app_name}" >> "$output"
echo "Icon=${icon}" >> "$output"
echo "Terminal=false" >> "$output"
echo "Categories=Development" >> "$output"

chmod a+x "$app"
chmod a+x "$output"

echo "generate ${output} ok."
