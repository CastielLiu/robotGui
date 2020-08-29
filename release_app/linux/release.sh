
app_name=robotGui
app_dir=`pwd`/pkg
app=$app_dir/${app_name}

linuxdeployqt ${app} -appimage

rm $app_dir/default.desktop
rm $app_dir/default.png

cp -r `pwd`/../../command `pwd`
cp -r `pwd`/../../goals `pwd`
cp -r `pwd`/../../icon `pwd`/pkg



