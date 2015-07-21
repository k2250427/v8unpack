Для создания вручную rpm пакета необходимо выполнить 

```
sudo dnf install fedora-packager @development-tools
sudo dnf install ligstdc++-devel, boost-devel, zlib-devel

```

создаем окружение 

```
rpmdev-setuptree
```

Копируем v8unpack.spec в ~/rpmbuild/SPEC
переименовать каталог v8unpack в v8unpack-3.0 и запаковать его в v8unpack-3.0.tar.gz
скопировать v8unpack-3.0.tar.gz в SOURCE

пакет собирается с помощью команды 
```
rpmbuild -bi ~/rpmbuild/SPEC/v8unpack.spec

```

