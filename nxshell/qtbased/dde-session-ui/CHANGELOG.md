<a name="4.9.12"></a>
### 4.9.12 (2019-06-25)


#### Bug Fixes

* **shutdown:**  not hide hibernate button when show again ([56ecf7ec](https://github.com/linuxdeepin/dde-session-ui/commit/56ecf7ec5d1f8dc2f3008f04aa29b557bd50a898))



<a name="4.9.11"></a>
### 4.9.11 (2019-06-13)


#### Bug Fixes

* **lock:**  unlock when fake nopassword user ([b4ef5413](https://github.com/linuxdeepin/dde-session-ui/commit/b4ef5413a0a0b577fe174d73715abb18958e250b))



<a name="4.9.10"></a>
### 4.9.10 (2019-06-11)


#### Bug Fixes

*   replace deepin_qt to dtk_qmake ([97ecea1f](https://github.com/linuxdeepin/dde-session-ui/commit/97ecea1f2f49b7fbba5752b02dc8bc64631f6739))
* **lock:**  fake nopassword user when user not have password ([11f49929](https://github.com/linuxdeepin/dde-session-ui/commit/11f499295e82c18d06b2655728c986c31abe7948))



<a name="4.9.9"></a>
### 4.9.9 (2019-06-04)


#### Bug Fixes

* **greeter:**  update user list ([fd4c40de](https://github.com/linuxdeepin/dde-session-ui/commit/fd4c40de0540b6aa734aff21d8f204c73245818c))
* **shutdown:**  solve the problem caused by rebase again ([6fd6c2df](https://github.com/linuxdeepin/dde-session-ui/commit/6fd6c2df81a82938124b2c43da5cdbd5ebe5db84))



<a name="4.9.8"></a>
### 4.9.8 (2019-06-01)


#### Bug Fixes

*   user avatar not show ([1013622f](https://github.com/linuxdeepin/dde-session-ui/commit/1013622ffd8f9f4fb3c7215904c55921cb99b6f6))
* **greeter:**
  *  missing blurdone signal ([5e3e9a6d](https://github.com/linuxdeepin/dde-session-ui/commit/5e3e9a6d408839787f1e28fb954fe1339f3d2554))
  *  wrong set user keyboard layout ([d2e00af5](https://github.com/linuxdeepin/dde-session-ui/commit/d2e00af5a7e3ee52b365528333efbe9267d272e4))
  *  not show fprint pam info ([731be30b](https://github.com/linuxdeepin/dde-session-ui/commit/731be30b2a46f642deb32b375b76aa9238c6633a))
* **lock:**
  *  when user unlock not reset checkpasswd num ([ae17a8a0](https://github.com/linuxdeepin/dde-session-ui/commit/ae17a8a0e783efde4a760f86f5a679c1c31a2222))
  *  cannot reboot system ([93350745](https://github.com/linuxdeepin/dde-session-ui/commit/933507456e9bc52932fbadf5243001e729217c74))
* **shutdown:**
  *  when inhibitwarningview show after system monitor button cannot enter hover state ([5f45073d](https://github.com/linuxdeepin/dde-session-ui/commit/5f45073d78d4e9cafada85b4e6961958e9731d34))
  *  multiuser warning tips are truncated ([ca312cde](https://github.com/linuxdeepin/dde-session-ui/commit/ca312cde8128a8dda567f80f37e96a8a6137155c))

#### Features

*   create document ([9f788c9b](https://github.com/linuxdeepin/dde-session-ui/commit/9f788c9b8fb8b645974b085032af8096fa159d2c))
*   split config files ([19a679db](https://github.com/linuxdeepin/dde-session-ui/commit/19a679db2dfa75072b1ce2fed49b3cb6c0764235))
* **greeter:**
  *  support not init native user ([a4e4cb59](https://github.com/linuxdeepin/dde-session-ui/commit/a4e4cb5945f26eaf321a9ca20f11037cd33d54c1))
  *  init LDAP user is first ([2c7d8456](https://github.com/linuxdeepin/dde-session-ui/commit/2c7d8456ec1c4b4a750ced06de127195f03b64c5))
  *  Add ShowSwitchUserButton by [always|ondemand|disabled] ([f8b23bae](https://github.com/linuxdeepin/dde-session-ui/commit/f8b23bae53a3a6174ae703cf0ddecea6596f2bd5))
  *  Add login box does not display avatar ([bf2ff638](https://github.com/linuxdeepin/dde-session-ui/commit/bf2ff638ed4164db9aeef355bb4651324c815a79))
* **lock:**  add LDAP login switch button ([4912e8dc](https://github.com/linuxdeepin/dde-session-ui/commit/4912e8dcf6a4e66dc628df2d9a6be4aaa461a57f))
* **osd:**  support set monitor config when release meta key ([4ba7e2b1](https://github.com/linuxdeepin/dde-session-ui/commit/4ba7e2b16db54559bc587b830d44782e259aba1d))



<a name="4.9.7"></a>
### 4.9.7 (2019-05-10)


#### Bug Fixes

*   crash in refreshInputState when m_user is null (#98) ([aa52dc67](https://github.com/linuxdeepin/dde-session-ui/commit/aa52dc67cc4d99061e41c30ecc04c8d45025f2aa))
* **greeter:**  not select user last use session ([971ce9ac](https://github.com/linuxdeepin/dde-session-ui/commit/971ce9ac2d1f11609a846de798ada1c3018a915a))



<a name="4.9.6"></a>
### 4.9.6 (2019-04-26)


#### Bug Fixes

* **greeter:**  Prevent failures caused by authentication at startup ([20483a28](https://github.com/linuxdeepin/dde-session-ui/commit/20483a28724fdb8825dec314a054e7daf4fb00d9))



<a name="4.9.5"></a>
### 4.9.5 (2019-04-24)


#### Bug Fixes

* **greeter:**  session list position not align center ([480c1b7c](https://github.com/linuxdeepin/dde-session-ui/commit/480c1b7c7e0d46b6cfbff442b6d98f7b466026fd))
* **lock:**
  *  No focus after closing login lock ([058269dd](https://github.com/linuxdeepin/dde-session-ui/commit/058269ddc0fdd31209df3ccb5e148f448c5c9996))
  *  not auth user when show again ([bc377453](https://github.com/linuxdeepin/dde-session-ui/commit/bc377453b026351de86027cc587e5bacc65f3cc4))
* **shutdown:**  crash when show inhibitwarnview again ([00efa492](https://github.com/linuxdeepin/dde-session-ui/commit/00efa492e92c59bce69b61d16e8377093a46a1d7))



<a name="4.9.4"></a>
### 4.9.4 (2019-04-22)


#### Bug Fixes

* **greeter:**  missing unlock mutex ([6730d29f](https://github.com/linuxdeepin/dde-session-ui/commit/6730d29f615f423e3b3d25eaf3f4c9e49585f347))
* **lock:**
  *  not sync multi-screen user lock info ([df0e04a7](https://github.com/linuxdeepin/dde-session-ui/commit/df0e04a7cd630456a98fd920ad8dada7d235a461))
  *  can always auth user ([e72070fe](https://github.com/linuxdeepin/dde-session-ui/commit/e72070fedb4a0c7ad8150d9f3247fdd484f27788))
  *  show error at first time ([d9eab194](https://github.com/linuxdeepin/dde-session-ui/commit/d9eab1942f9b0e3c414218620d0bcb38537c5d9d))
  *  shutdown fprintd device for the first time ([45140c17](https://github.com/linuxdeepin/dde-session-ui/commit/45140c170371b26341d199a61190847923445c39))



<a name="4.9.3"></a>
### 4.9.3 (2019-04-17)


#### Features

* **greeter:**  use new dpi scale ratio config ([f4ca6a82](https://github.com/linuxdeepin/dde-session-ui/commit/f4ca6a8220a9c9e94565af242c506b0549694f7a))

#### Bug Fixes

* **lock:**
  *  nopasswdlogin cannot unlock ([4568763f](https://github.com/linuxdeepin/dde-session-ui/commit/4568763fa9d79ac995bac09531fe6b80355a8424))
  *  not refresh nopasswordlogin when show again ([029157ed](https://github.com/linuxdeepin/dde-session-ui/commit/029157ed7b3ea163d43d4d29bc67e54054ac043d))



<a name="4.9.2"></a>
### 4.9.2 (2019-04-09)


#### Features

*   support deepin-auth ([07a3f29f](https://github.com/linuxdeepin/dde-session-ui/commit/07a3f29f830d8a242221574b90fc3022a20bb408))
* **greeter:**  support multi screen scale ratio ([8e8136e0](https://github.com/linuxdeepin/dde-session-ui/commit/8e8136e0215a236deeebdd235ed3fd1d081f758e))



<a name="4.9.1"></a>
### 4.9.1 (2019-04-02)


#### Bug Fixes

*   missing deepin-qtconfig ([f13795b0](https://github.com/linuxdeepin/dde-session-ui/commit/f13795b016554a024fe297bdeb9fe5af9578ba45))
*   not check swap again ([e7b5865c](https://github.com/linuxdeepin/dde-session-ui/commit/e7b5865cc6ada519cf3e9c6956c7f8490f2d5470))



<a name=""></a>
##  4.9.0 (2019-03-26)


#### Bug Fixes

*   screen copy mode will empty page ([697db101](https://github.com/linuxdeepin/dde-session-ui/commit/697db1012258b54f3a312bdc35789861a47943e3))
*   missing memory header ([1c987fb7](https://github.com/linuxdeepin/dde-session-ui/commit/1c987fb73e043ca7c8e117690f5ed355a5c7a7ce))
* **dde-lowpower:**  missing c++11 cxxflags ([f7f23b23](https://github.com/linuxdeepin/dde-session-ui/commit/f7f23b2384cedd8ffcb2cb28552c3c643f99ec82))
* **greeter:**  mulit-user fingerprint login ([cc15f58c](https://github.com/linuxdeepin/dde-session-ui/commit/cc15f58cce0c61091446f1600bc0713f5f3f905f))
* **shutdown:**
  *  multi-screen data sync ([4178fd5f](https://github.com/linuxdeepin/dde-session-ui/commit/4178fd5fcc68535aa31796d463321685b9e8577d))
  *  translation error ([bcc49bc9](https://github.com/linuxdeepin/dde-session-ui/commit/bcc49bc973e54dd49b1eb9ee22be304123e44e8b))

#### Features

*   lock and greeter support disable sleep button for shutdownpage ([46d77627](https://github.com/linuxdeepin/dde-session-ui/commit/46d77627fbc6f15675cda9a41456e52679533e80))
*   support config file to control function ([b2d41855](https://github.com/linuxdeepin/dde-session-ui/commit/b2d41855a2a1b6ea2ef3459bd1661170e53100fa))
* **shutdown:**
  *  support use config value to control sleep button ([40c124c4](https://github.com/linuxdeepin/dde-session-ui/commit/40c124c474dc51188230e4711a9f45ae3c1b9d6e))
  *  inhibitwarning support data sync ([a15388da](https://github.com/linuxdeepin/dde-session-ui/commit/a15388da3e2b818ed1a086f2ac95cd209fa222d8))



<a name="4.8.13"></a>
### 4.8.13 (2019-03-08)


#### Bug Fixes

*   not need restart the process when workareachanged ([06a662cc](https://github.com/linuxdeepin/dde-session-ui/commit/06a662cc4f7bf1546cef04350bdac215e3b14568))
* **shutdown:**  switch user button will show when show frame again ([6276ebd6](https://github.com/linuxdeepin/dde-session-ui/commit/6276ebd622a790b0be2c3dd24d8b1faa3cf951c8))



<a name="4.8.12"></a>
### 4.8.12 (2019-03-06)


#### Bug Fixes

* **greeter:**  when have addoman login, cannot use fprintd login ([f1721cf5](https://github.com/linuxdeepin/dde-session-ui/commit/f1721cf58be424870ca073d72c286cb29542406f))
* **lock:**
  *  virtualKeyboard show need twice click ([5db3a08e](https://github.com/linuxdeepin/dde-session-ui/commit/5db3a08e146980fbfa28fc67c01cd8a38112fcae))
  *  switch user button not change when user list changed ([b037b013](https://github.com/linuxdeepin/dde-session-ui/commit/b037b013ee85e86926a623818a826774dc6fe395))
* **shutdown:**  switch user do not need to display ([e04d743c](https://github.com/linuxdeepin/dde-session-ui/commit/e04d743c041ee69c39707c89a5089cc695012d04))



<a name="4.8.11"></a>
### 4.8.11 (2019-02-25)


#### Bug Fixes

* **Shutdown:**  don't show self in multiUserWarning when poweroff ([3762c144](https://github.com/linuxdeepin/dde-session-ui/commit/3762c144d109b08445b40c7184afb616e303fb22))
* **greeter:**  password veification when using a fingerprint that interface will always running verification animation ([45962694](https://github.com/linuxdeepin/dde-session-ui/commit/4596269481ee9cdc60f2d850f96e4826aad0e8c5))
* **lock:**
  *  not delete nonexistent user ([adf47c8d](https://github.com/linuxdeepin/dde-session-ui/commit/adf47c8dfccdcd930084661f66f3320de7c2242a))
  *  wrong user list check ([564e0c88](https://github.com/linuxdeepin/dde-session-ui/commit/564e0c88070a077ac3518595b69d321bda6ae3bb))
  *  not refresh avatar and display name when change user info ([b056b7d1](https://github.com/linuxdeepin/dde-session-ui/commit/b056b7d1824e52d9882e0b6d9549375bfded3f5d))
* **shutdown:**  multiuser warning view not refresh user list ([5c80a9d2](https://github.com/linuxdeepin/dde-session-ui/commit/5c80a9d2317661aa5eaaa56fe1ff53c6c9fa6d2f))



<a name="4.8.10"></a>
### 4.8.10 (2019-02-18)


#### Bug Fixes

* **shutdown:**  missing daemon option ([fb5d5ee1](https://github.com/linuxdeepin/dde-session-ui/commit/fb5d5ee1cb4aaceb455198b2f3778ee35a88e7b6))



<a name="4.8.9"></a>
### 4.8.9 (2019-02-15)




<a name="4.8.8"></a>
### 4.8.8 (2019-02-15)


#### Bug Fixes

*   crash when running lightdm-deepin-greeter ([f20ad3ec](https://github.com/linuxdeepin/dde-session-ui/commit/f20ad3ec8222ffa5168eae8931edcc3f0ac87d08))



<a name="4.8.7"></a>
### 4.8.7 (2019-01-25)


#### Bug Fixes

*   keep the same amount as the screen ([fb5868b8](https://github.com/linuxdeepin/dde-session-ui/commit/fb5868b81dabb23e4b8395caaf85b1d892692b02))
* **lock:**  not load translate for login button ([50d59da7](https://github.com/linuxdeepin/dde-session-ui/commit/50d59da76cb43d9cda53f2e966ddbae3cbec996d))

#### Features

* **Shutdown:**  add Hibernate for dbus interface ([6ae3014d](https://github.com/linuxdeepin/dde-session-ui/commit/6ae3014d12006d82306a6d1d267d9b1f9b40d5a7))



<a name="4.8.6"></a>
### 4.8.6 (2019-01-15)


#### Features

* **Lock:**  support enter key to login system ([bc4e2823](https://github.com/linuxdeepin/dde-session-ui/commit/bc4e282336ef84c212346a5bc826f98f34252085))



<a name="4.8.5"></a>
### 4.8.5 (2019-01-14)


#### Bug Fixes

*   libnotify freeze when calling dbus notify ([e49fb176](https://github.com/linuxdeepin/dde-session-ui/commit/e49fb17663dfadbcd292bbd1cbb7955a36cbfe90))
* **Lock:**  nopassword user cannot switchToUser ([e2219d3b](https://github.com/linuxdeepin/dde-session-ui/commit/e2219d3b6ee2b404178e1e85e649a3ac727bc4d0))
* **Shutdown:**  confirm page not show ([07df6e41](https://github.com/linuxdeepin/dde-session-ui/commit/07df6e41e39ce1acf519235df7ca414f0ca45520))
* **greeter:**
  *  wrong num lock state when laptop not use battery ([78971b15](https://github.com/linuxdeepin/dde-session-ui/commit/78971b15e46b87581721a0dd49d2769f7c307bf1))
  *  unlock with fingerprint will crash ([a824c8e7](https://github.com/linuxdeepin/dde-session-ui/commit/a824c8e77c6cdbfbce541fcb06939b245970975f))



<a name="4.8.4"></a>
### 4.8.4 (2019-01-10)


#### Bug Fixes

*   no messages when using fingerprint verification ([c2b5a782](https://github.com/linuxdeepin/dde-session-ui/commit/c2b5a782fa951f63c7d24df9875ded9fe4d0b5dd))
*   do not start user auth on the dde-shutdown application ([f246b1d1](https://github.com/linuxdeepin/dde-session-ui/commit/f246b1d15ef9d69766525e366a09574541b297e2))
*   the keyboard list widget can not fully display ([f76cdd18](https://github.com/linuxdeepin/dde-session-ui/commit/f76cdd1881b08c5961775ff6586fc7ea29bca3ea))
*   the widgets can not fully display on the hi-DPI screen ([b56d624d](https://github.com/linuxdeepin/dde-session-ui/commit/b56d624d35a5fb9ce88231f06061e8deb7c33bf1))
*   hide the user switch icon if only have one user ([3d6c915d](https://github.com/linuxdeepin/dde-session-ui/commit/3d6c915d4b5e59dddec8736b7e6dde922bbdde49))
*   wm mode chooser window size ([3e218a07](https://github.com/linuxdeepin/dde-session-ui/commit/3e218a074733f0f5c98e0dade56863c98c5ea54e))
*   the lock contents widget is hidden when move cursor to the screen corners ([7db08a83](https://github.com/linuxdeepin/dde-session-ui/commit/7db08a831e1ad3a470cb46796ad302c7f593625e))
*   warning "Unknow property font-stretch" ([6f407b2f](https://github.com/linuxdeepin/dde-session-ui/commit/6f407b2f9a207b11a82f38c8566f62440ad24af9))
* **greeter:**  no need to verify password ([e739c7dd](https://github.com/linuxdeepin/dde-session-ui/commit/e739c7dd6f791a483ffba98dedd12240ccc2573f))



<a name="4.8.3"></a>
## 4.8.3 (2019-01-09)


#### Bug Fixes

* **dde-welcome:**  geometry error ([cea5d637](https://github.com/linuxdeepin/dde-session-ui/commit/cea5d6379acd3714f0fa1f26b8aec7d087cea0eb))
* **greeter:**
  *  shutdown and reboot don't auth user ([ed486330](https://github.com/linuxdeepin/dde-session-ui/commit/ed486330ad5d38651ad3a4f2019cc334580a71cd))
  *  check user is nopasswordgrp ([f0d2278b](https://github.com/linuxdeepin/dde-session-ui/commit/f0d2278b46cacadd9d0f5d6221992c03c9bd137b))
  *  not auth current user ([71786180](https://github.com/linuxdeepin/dde-session-ui/commit/71786180717ecc9a7dba26e6efbeebf0bcc6ce14))
  *  cannot show virtualKB ([f0aafb63](https://github.com/linuxdeepin/dde-session-ui/commit/f0aafb634db0a5b0d6ffa103e2089a11cfbe29be))
  *  the deepin version lable not fully displayed ([c191e738](https://github.com/linuxdeepin/dde-session-ui/commit/c191e73828b680c99173b182cdd522b431b3a979))



<a name=""></a>
##  4.8.2 (2019-01-03)


#### Bug Fixes

* **Lock:**  crash when init virtual keyboard too slow ([d54fa4b2](https://github.com/linuxdeepin/dde-session-ui/commit/d54fa4b2b1a5c62f98e56075f343636051bdb966))
* **shutdown:**  switch to the right to crash. ([3c25fee9](https://github.com/linuxdeepin/dde-session-ui/commit/3c25fee9b12eed659a42a559eda8f23af5025210))



<a name="4.8.1"></a>
### 4.8.1 (2019-01-02)


#### Bug Fixes

*   version description not follow user ([f468cf1c](https://github.com/linuxdeepin/dde-session-ui/commit/f468cf1c03097136d4b9314e8ff7fb0ae7ec7347))

#### Features

*   start auth user at startup ([1b2503b0](https://github.com/linuxdeepin/dde-session-ui/commit/1b2503b06c0021200dca9304ee6a740233b9b614))
* **greeter:**  logo widget remove fixed size ([54796afd](https://github.com/linuxdeepin/dde-session-ui/commit/54796afdd237d20e11ede18dbe0d1df7f084cbc8))



<a name=""></a>
##  4.8.0 (2018-12-25)


#### Bug Fixes

*   wrong cursor pos ([81232cef](https://github.com/linuxdeepin/dde-session-ui/commit/81232cef4baadbd22e62d6878969ecd6dd65abcf))
*   update default background path ([c85ab6cd](https://github.com/linuxdeepin/dde-session-ui/commit/c85ab6cdfe297f744080a8b3429ebc5b2797de87))
*   wrong init addomain user ([87a70761](https://github.com/linuxdeepin/dde-session-ui/commit/87a70761125a4f4b7ebbaa40a6c271ae1d22f6a7))
*   hybird-sleep to suspend ([fd32d0ff](https://github.com/linuxdeepin/dde-session-ui/commit/fd32d0ff1be90b0fefdc26cddf3b2c00543a8827))
*   switch user will reset lock time ([aa72aa24](https://github.com/linuxdeepin/dde-session-ui/commit/aa72aa24c3bc9d6557b363b9ea02e36a0ddda9e5))
*   cannot switch mode ([30ff00a8](https://github.com/linuxdeepin/dde-session-ui/commit/30ff00a884750d79eed84707eec2a85db5565dec))
*   remove user list sort ([de1dedd4](https://github.com/linuxdeepin/dde-session-ui/commit/de1dedd421dacf20461bc33c23c85ca5210544b3))
*   move the screen to another screen to hide invalid tip. ([49bc78fe](https://github.com/linuxdeepin/dde-session-ui/commit/49bc78feb2458ead4d91c97fd5218bf05fed6b70))
* **DNetworkSecretDialog:**  submit on enter key pressed ([ce56d706](https://github.com/linuxdeepin/dde-session-ui/commit/ce56d7064e2252393bc423205465b2cd00259373))
* **Lock:**  cannot hibernate ([a2a106cf](https://github.com/linuxdeepin/dde-session-ui/commit/a2a106cfeb4ed5682b42bac809daadf36d2fabb5))
* **Power:**  use hybrid sleep replace suspend ([f9266a4d](https://github.com/linuxdeepin/dde-session-ui/commit/f9266a4d21fe8ed1d24d5f9aba7a15a0cac74829))
* **Shutdown:**  not support click hibernate button ([06c353b1](https://github.com/linuxdeepin/dde-session-ui/commit/06c353b161903b044e39bc81bb608fd208a4ea46))
* **UserFrame:**
  *  cannot scroll frame ([1502f73d](https://github.com/linuxdeepin/dde-session-ui/commit/1502f73d9bb7cca7bb1349af2861962236137c79))
  *  not set logind icon ([08cdf3d8](https://github.com/linuxdeepin/dde-session-ui/commit/08cdf3d8ffbc28f924d2aa6747f69bf1d45ce36a))
  *  cannot push userframe again ([43b8329f](https://github.com/linuxdeepin/dde-session-ui/commit/43b8329fb51030caeef72792d630cdafc546a66c))
* **background:**  draw background shift when scale ratio ([109603d4](https://github.com/linuxdeepin/dde-session-ui/commit/109603d423600ab54d86caae3f7b8e0fce382f47))
* **greeter:**
  *  cannot login ([421e7ce9](https://github.com/linuxdeepin/dde-session-ui/commit/421e7ce9b70953a5b84dbd2b6c442c2c04717c95))
  *  not set current user when only one user exist ([38ff711d](https://github.com/linuxdeepin/dde-session-ui/commit/38ff711d221c7957a28dccf7ecd89677f17f24a6))
  *  not set current user ([4f60f513](https://github.com/linuxdeepin/dde-session-ui/commit/4f60f5133f3ef019f4cc3fea7052a0d9e7e3d280))
* **lock:**  cancel power action for click empty region ([7264f728](https://github.com/linuxdeepin/dde-session-ui/commit/7264f728ac233f69b56392cdf7c3d83169f814bb))
* **osd:**
  *  not set prefix ([3122776b](https://github.com/linuxdeepin/dde-session-ui/commit/3122776b2f4a34eacf3d070a40797f9c25a93f58))
  *  wrong return value ([b69372ad](https://github.com/linuxdeepin/dde-session-ui/commit/b69372adbf37a7a3b2471296358ade951f7b9369))
* **session:**  hide control when only one session ([db7470b6](https://github.com/linuxdeepin/dde-session-ui/commit/db7470b657518850e6ce6ad97a1fa33e935c0fee))
* **shutdown:**  switch user not visible ([6fa6b3e3](https://github.com/linuxdeepin/dde-session-ui/commit/6fa6b3e39c46142189ad126e05dd56c75df04130))

#### Features

*   update style for login ([7f81e9d0](https://github.com/linuxdeepin/dde-session-ui/commit/7f81e9d089b9afc2680ee2e985a3d45a273ae7e8))
*   add doaction dbus interface ([07fe1b7a](https://github.com/linuxdeepin/dde-session-ui/commit/07fe1b7af9c3c3a1344295a0e357293ad4bbd87f))
*   useless openssh-server ([beab6602](https://github.com/linuxdeepin/dde-session-ui/commit/beab66027a36e05297538cc43c7977f26ee75221))
*   change default state from hover to checked ([1f1aedad](https://github.com/linuxdeepin/dde-session-ui/commit/1f1aedadb44657f89d01f534a2ae6b478f42f6cf))
* **Greeter:**  add ad user login ([ca8a9f46](https://github.com/linuxdeepin/dde-session-ui/commit/ca8a9f46fe81b565d45c47eaf0d1c9a77c8cf490))
* **Lock:**
  *  restore user password when switch user ([9c8c3bf5](https://github.com/linuxdeepin/dde-session-ui/commit/9c8c3bf546fb4180c174efda35cb5985f70a1435))
  *  add verification limit ([8425cf66](https://github.com/linuxdeepin/dde-session-ui/commit/8425cf66697817a89fac38eebf811afaedab2c38))
  *  add hibernate ([5045d861](https://github.com/linuxdeepin/dde-session-ui/commit/5045d8615f40664dfafcf6f7e2ce9148dc6380f3))
  *  add lock time for auth failed ([0182b0fe](https://github.com/linuxdeepin/dde-session-ui/commit/0182b0fe51c030858a417028abcef97d25ae678c))
  *  add hibernate ([c506e4a9](https://github.com/linuxdeepin/dde-session-ui/commit/c506e4a959a97dcc93544dfd8f87b93a2b2b4fa2))
  *  add virtual keyboard ([650f9366](https://github.com/linuxdeepin/dde-session-ui/commit/650f9366681ac9125cc280185d8be088e5f30ee8))
* **Shutdown:**
  *  update system monitor button style ([785f9f59](https://github.com/linuxdeepin/dde-session-ui/commit/785f9f59ed1a1b8d0880814be4a1fb1e22d141ac))
  *  add hibernate ([082bee36](https://github.com/linuxdeepin/dde-session-ui/commit/082bee36455e72e7773991ba2778578c54357a8c))
* **session:**  support for left & right key selection. ([ef4ca905](https://github.com/linuxdeepin/dde-session-ui/commit/ef4ca905921f1db6684f2a41a50ddf0b46518aba))



<a name=""></a>
##  4.7.0 (2018-12-07)


#### Bug Fixes

*   cannot switch mode ([30ff00a8](https://github.com/linuxdeepin/dde-session-ui/commit/30ff00a884750d79eed84707eec2a85db5565dec))
*   remove user list sort ([de1dedd4](https://github.com/linuxdeepin/dde-session-ui/commit/de1dedd421dacf20461bc33c23c85ca5210544b3))
*   move the screen to another screen to hide invalid tip. ([49bc78fe](https://github.com/linuxdeepin/dde-session-ui/commit/49bc78feb2458ead4d91c97fd5218bf05fed6b70))
* **DNetworkSecretDialog:**  submit on enter key pressed ([ce56d706](https://github.com/linuxdeepin/dde-session-ui/commit/ce56d7064e2252393bc423205465b2cd00259373))
* **Power:**  use hybrid sleep replace suspend ([f9266a4d](https://github.com/linuxdeepin/dde-session-ui/commit/f9266a4d21fe8ed1d24d5f9aba7a15a0cac74829))
* **Shutdown:**  not support click hibernate button ([06c353b1](https://github.com/linuxdeepin/dde-session-ui/commit/06c353b161903b044e39bc81bb608fd208a4ea46))
* **UserFrame:**
  *  cannot scroll frame ([1502f73d](https://github.com/linuxdeepin/dde-session-ui/commit/1502f73d9bb7cca7bb1349af2861962236137c79))
  *  not set logind icon ([08cdf3d8](https://github.com/linuxdeepin/dde-session-ui/commit/08cdf3d8ffbc28f924d2aa6747f69bf1d45ce36a))
  *  cannot push userframe again ([43b8329f](https://github.com/linuxdeepin/dde-session-ui/commit/43b8329fb51030caeef72792d630cdafc546a66c))
* **background:**  draw background shift when scale ratio ([109603d4](https://github.com/linuxdeepin/dde-session-ui/commit/109603d423600ab54d86caae3f7b8e0fce382f47))
* **greeter:**
  *  cannot login ([421e7ce9](https://github.com/linuxdeepin/dde-session-ui/commit/421e7ce9b70953a5b84dbd2b6c442c2c04717c95))
  *  not set current user when only one user exist ([38ff711d](https://github.com/linuxdeepin/dde-session-ui/commit/38ff711d221c7957a28dccf7ecd89677f17f24a6))
  *  not set current user ([4f60f513](https://github.com/linuxdeepin/dde-session-ui/commit/4f60f5133f3ef019f4cc3fea7052a0d9e7e3d280))
* **osd:**  wrong return value ([b69372ad](https://github.com/linuxdeepin/dde-session-ui/commit/b69372adbf37a7a3b2471296358ade951f7b9369))
* **shutdown:**  switch user not visible ([6fa6b3e3](https://github.com/linuxdeepin/dde-session-ui/commit/6fa6b3e39c46142189ad126e05dd56c75df04130))

#### Features

*   change default state from hover to checked ([1f1aedad](https://github.com/linuxdeepin/dde-session-ui/commit/1f1aedadb44657f89d01f534a2ae6b478f42f6cf))
* **Lock:**
  *  add verification limit ([8425cf66](https://github.com/linuxdeepin/dde-session-ui/commit/8425cf66697817a89fac38eebf811afaedab2c38))
  *  add hibernate ([5045d861](https://github.com/linuxdeepin/dde-session-ui/commit/5045d8615f40664dfafcf6f7e2ce9148dc6380f3))
  *  add lock time for auth failed ([0182b0fe](https://github.com/linuxdeepin/dde-session-ui/commit/0182b0fe51c030858a417028abcef97d25ae678c))
  *  add hibernate ([c506e4a9](https://github.com/linuxdeepin/dde-session-ui/commit/c506e4a959a97dcc93544dfd8f87b93a2b2b4fa2))
  *  add virtual keyboard ([650f9366](https://github.com/linuxdeepin/dde-session-ui/commit/650f9366681ac9125cc280185d8be088e5f30ee8))
* **Shutdown:**  add hibernate ([082bee36](https://github.com/linuxdeepin/dde-session-ui/commit/082bee36455e72e7773991ba2778578c54357a8c))
* **session:**  support for left & right key selection. ([ef4ca905](https://github.com/linuxdeepin/dde-session-ui/commit/ef4ca905921f1db6684f2a41a50ddf0b46518aba))



<a name="4.6.2"></a>
## 4.6.2 (2018-11-12)


#### Bug Fixes

*   icon size in hihdpi mode of network secret dialog ([532742f6](https://github.com/linuxdeepin/dde-session-ui/commit/532742f67bd818d16dea9f6a328e2b198ee1af26))



<a name="4.6.1"></a>
### 4.6.1 (2018-11-09)


#### Bug Fixes

*   insufficient button space ([afc977a9](https://github.com/linuxdeepin/dde-session-ui/commit/afc977a9d4ae1ade1219c1422e47e77388e6ff64))
*   checked Mark support hidpi. ([961dc66f](https://github.com/linuxdeepin/dde-session-ui/commit/961dc66fdf9ad29d2f37ef8ef761681646a86fcf))
*   not restore user password when user changed ([4aa6e81e](https://github.com/linuxdeepin/dde-session-ui/commit/4aa6e81eeb5e827e77e80888a7afa2fbc765f9cf))
*   hihdpi icon of lineedit clear button ([f078379e](https://github.com/linuxdeepin/dde-session-ui/commit/f078379eab6bbcdf0e2318d1b8591c826f881693))
* **greeter:**  not use user's locale language ([42505736](https://github.com/linuxdeepin/dde-session-ui/commit/42505736d83e2ef30dc752d4b08a2001210e78e7))
* **welcome:**  not have keypress event ([6b1fb901](https://github.com/linuxdeepin/dde-session-ui/commit/6b1fb90146b73d2a414836bd356af842cf5a6a1d))

#### Features

* **greeter:**  changle root cursor when start session ([8c3fa4f2](https://github.com/linuxdeepin/dde-session-ui/commit/8c3fa4f27520258436208de89c1ad817f09a7462))



<a name=""></a>
##  4.6.0 (2018-10-25)


#### Features

*   disable unlock animation for mips and sw ([39c23818](https://github.com/linuxdeepin/dde-session-ui/commit/39c23818270f84940ec9e32df2fc35355f974980))
*   update judgement rule ([3783095b](https://github.com/linuxdeepin/dde-session-ui/commit/3783095be9a8f528913df5101f702869c627c3bc))
*   use shell script replace xrandr handle for main.cpp ([b2d4828c](https://github.com/linuxdeepin/dde-session-ui/commit/b2d4828c89c65e33508fa5be4a7d901251ceb9ef))
* **OSD:**  add trackpad toggle svg ([24e525cc](https://github.com/linuxdeepin/dde-session-ui/commit/24e525cc019e97ba49bebbbd48acd4bc08ef19aa))
* **greeter:**
  *  support run some scripts ([2f3a3e31](https://github.com/linuxdeepin/dde-session-ui/commit/2f3a3e31292f5e3a96a83e3484eb083f9d13e152))
  *  disable login animation for sw64 ([4c77610a](https://github.com/linuxdeepin/dde-session-ui/commit/4c77610a8d0267d3b690fdb3da7853107cb85255))
  *  save last login user and restore last logout user after startup ([9bfbbd9a](https://github.com/linuxdeepin/dde-session-ui/commit/9bfbbd9a3f5cc3120b789d4ced7557854609ad5b))
* **notify:**
  *  run only under the deepin desktop ([edf83b2a](https://github.com/linuxdeepin/dde-session-ui/commit/edf83b2a7fe7da82e4c70cb435eb541a9740b6ee))
  *  support desktop entry for notify hints ([1f247e8c](https://github.com/linuxdeepin/dde-session-ui/commit/1f247e8c6bda3d4bc4ed820f48ba8002932c39d6))
* **welcome:**  remove check system version ([aad78ad2](https://github.com/linuxdeepin/dde-session-ui/commit/aad78ad2530bc1ee827244aacde71573b5fa6352))

#### Bug Fixes

*   00-xrandr ([93d6655f](https://github.com/linuxdeepin/dde-session-ui/commit/93d6655f41e84b36163b0b39225abf5b89f050f6))
*   mission build depend ([021ab0cc](https://github.com/linuxdeepin/dde-session-ui/commit/021ab0cc439bf263f20d88fbb537414802db44ce))
*   add depend only on amd64 ([57632e6d](https://github.com/linuxdeepin/dde-session-ui/commit/57632e6d9dc3adda81388274a2318fea36c9dc1b))
*   move elf to /usr/bin/ ([58964dd1](https://github.com/linuxdeepin/dde-session-ui/commit/58964dd14bdd32c0b9fb9a88c0194a7807bd067e))
*   not set auth user after auth faild ([d13e46bf](https://github.com/linuxdeepin/dde-session-ui/commit/d13e46bf4c70b97404aa047b63d438099d67cb16))
*   typo ([af328672](https://github.com/linuxdeepin/dde-session-ui/commit/af328672f9ace5c99cdaa6fb1a7a0fb47148f7ea))
*   add depend only on amd64 ([ce392422](https://github.com/linuxdeepin/dde-session-ui/commit/ce39242227fa473347bebf99abfa63cde5bb7921))
*   non-essential service is required ([55cebdbc](https://github.com/linuxdeepin/dde-session-ui/commit/55cebdbc2ff9be06bf4de465050790c5798b8805))
* **Background:**
  *  background align center for screen ([9beebf3b](https://github.com/linuxdeepin/dde-session-ui/commit/9beebf3bcbef33031f68b44cb437eff29f7b6ad8))
  *  Error of precision ([509d2e3c](https://github.com/linuxdeepin/dde-session-ui/commit/509d2e3c9bb25342b0b3ffddc114baa3e5bd60d5))
* **KBLayout:**  missing setxkbmap model argument ([2cd1a738](https://github.com/linuxdeepin/dde-session-ui/commit/2cd1a7386762914802557cddf99efadd6dd4cb3d))
* **dde-lock:**  disable input method ([15ae6623](https://github.com/linuxdeepin/dde-session-ui/commit/15ae66230f70338177e264d20636d680e75a1e25))
* **greeter:**  nopasswd user cannot login ([12201b10](https://github.com/linuxdeepin/dde-session-ui/commit/12201b10526ca913a680a77ec1bd9dc397b829c1))
* **lock:**
  *  Error updating user status ([b38e492b](https://github.com/linuxdeepin/dde-session-ui/commit/b38e492b66256a193753e9452cac2827c87e2d0f))
  *  not hide alert ([a6cbd383](https://github.com/linuxdeepin/dde-session-ui/commit/a6cbd383343a9dfb57c3179687c13a3f74cb82d3))
  *  grab keyboard failure ([05281e22](https://github.com/linuxdeepin/dde-session-ui/commit/05281e22b39a651af900228b7718a0b12a9d99e8))
  *  crash when media player buttons is clicked ([a88533d3](https://github.com/linuxdeepin/dde-session-ui/commit/a88533d311bd26c0844ef7f730615c69dc9a3a43))
  *  not stop animation when unlock ([259f1f60](https://github.com/linuxdeepin/dde-session-ui/commit/259f1f606bb8ebcfa231fd8468259b7c8ce33577))
* **lock/lightdm:**  hide alert tooltip when shutdown widgets is displayed ([310160fd](https://github.com/linuxdeepin/dde-session-ui/commit/310160fd9f5fc025f36b6fd60a7cdfbc656c6f61))
* **shutdown:**  add background failback ([15e0c2c4](https://github.com/linuxdeepin/dde-session-ui/commit/15e0c2c4b2a2d9c7558bf18ed02eac77930de41d))



<a name=""></a>
##  4.5.1.11 (2018-10-24)


#### Bug Fixes

* **shutdown:**  background not split ([2f79d728](https://github.com/linuxdeepin/dde-session-ui/commit/2f79d728ce008c6b2133a2f1b02fa4c073e9906e))



<a name=""></a>
##  4.5.1.10 (2018-10-09)


#### Bug Fixes

*   compile failed on sw platform ([ed573c3a](https://github.com/linuxdeepin/dde-session-ui/commit/ed573c3ac717bc6041efabcb02c213cc669dfff6))



<a name=""></a>
##  4.5.1.9 (2018-10-08)


#### Bug Fixes

*   sw_64 missing host_sw_64 ([026883af](https://github.com/linuxdeepin/dde-session-ui/commit/026883afd56981b826fe4877e9a75dbd555ae013))



## 4.5.1.8 (2018-09-19)

* fix control build stable

<a name=""></a>
##  4.5.1.7 (2018-09-15)


#### Bug Fixes

*   move elf to /usr/bin/ ([d2c3ff0e](https://github.com/linuxdeepin/dde-session-ui/commit/d2c3ff0eed5a53d08c416d4fc3244dbafe07a226))

#### Features

*   use shell script replace xrandr handle for main.cpp ([5ae10a22](https://github.com/linuxdeepin/dde-session-ui/commit/5ae10a22153635c89921db9a82b7ce550d5cda57))



<a name=""></a>
##  4.5.1.6 (2018-09-13)


#### Bug Fixes

*   mission build depend ([62ebfdcf](https://github.com/linuxdeepin/dde-session-ui/commit/62ebfdcf8aa80dae67a251f039a30197d0a97db2))
* **greeter:**  nopasswd user cannot login ([0f4ff307](https://github.com/linuxdeepin/dde-session-ui/commit/0f4ff307015d649ef7df185521876939ba1dc0fa))

#### Features

*   disable unlock animation for mips and sw ([7f226572](https://github.com/linuxdeepin/dde-session-ui/commit/7f226572bc119a3873695df928eeee688c366484))
* **greeter:**  support run some scripts ([fe5c4e48](https://github.com/linuxdeepin/dde-session-ui/commit/fe5c4e48fa48e28d8fe3851d06917838acd8280e))



<a name=""></a>
##  4.5.1.5 (2018-09-11)


#### Features

*   update judgement rule ([dcbe3949](https://github.com/linuxdeepin/dde-session-ui/commit/dcbe3949bdd5137da0fe653dd4c4a67efe15d26c))
* **greeter:**  disable login animation for sw64 ([9522ca91](https://github.com/linuxdeepin/dde-session-ui/commit/9522ca913c06a3e0293095bd49c2d6a15dcbf56b))



<a name=""></a>
##  4.5.1.4 (2018-09-07)


#### Bug Fixes

*   not set auth user after auth faild ([be048683](https://github.com/linuxdeepin/dde-session-ui/commit/be0486835cbf9de61212ec3a2c0009174234519f))



<a name=""></a>
##  4.5.1.3 (2018-09-06)


#### Bug Fixes

* **Background:**  background align center for screen ([23cd8d51](https://github.com/linuxdeepin/dde-session-ui/commit/23cd8d51f4324bf88c65c1b1bac324338e679037))



<a name=""></a>
##  4.5.1.2 (2018-08-31)


#### Features

* **OSD:**  add trackpad toggle svg ([20231add](https://github.com/linuxdeepin/dde-session-ui/commit/20231add6ffb5bb5cafb1367a03ce5f1c83cde93))

#### Bug Fixes

*   add depend only on amd64 ([dbcc5a73](https://github.com/linuxdeepin/dde-session-ui/commit/dbcc5a73f6aa22828df2c18f016e0d45608f2c60))
* **dde-lock:**  disable input method ([c2aa0aed](https://github.com/linuxdeepin/dde-session-ui/commit/c2aa0aedcd34160d085ec3a1e478f3a5ec352ce8))
* **lock:**
  *  crash when media player buttons is clicked ([c020391c](https://github.com/linuxdeepin/dde-session-ui/commit/c020391c278922d6ebff4ea1bbb4b3d25d9ef638))
  *  grab keyboard failure ([a35906a2](https://github.com/linuxdeepin/dde-session-ui/commit/a35906a2e92f624538dbb4b4649e0c2b94d773bb))
  *  not stop animation when unlock ([9efa0f66](https://github.com/linuxdeepin/dde-session-ui/commit/9efa0f663fa5c2ab12a942972935275289494dbd))
* **lock/lightdm:**  hide alert tooltip when shutdown widgets is displayed ([5e6ba10c](https://github.com/linuxdeepin/dde-session-ui/commit/5e6ba10c13346cae8555e140eee3c3cd5a076a51))



<a name="4.5.1"></a>
### 4.5.1 (2018-08-14)




<a name=""></a>
##  4.5.0 (2018-08-12)


#### Bug Fixes

* **notify:**  pull dock immediately ([8da7ff6a](https://github.com/linuxdeepin/dde-session-ui/commit/8da7ff6abc899276090a486ac56f7ce74381b518))



<a name=""></a>
##  4.4.9.2 (2018-08-09)


#### Bug Fixes

*   wrong type comparison ([737c1a66](https://github.com/linuxdeepin/dde-session-ui/commit/737c1a66dbccf7b773d473f6f1cec85a0e991568))



<a name=""></a>
##  4.4.9.1 (2018-08-09)


#### Bug Fixes

*   wrong type comparison ([5ecc1dc9](https://github.com/linuxdeepin/dde-session-ui/commit/5ecc1dc966f668a57b4c0805917fb16b5fa3e35c))



<a name="4.4.9"></a>
### 4.4.9 (2018-08-07)


#### Features

* **osd:**  support failback to appname of notify ([372c83a6](https://github.com/linuxdeepin/dde-session-ui/commit/372c83a6bdbb504f06aee26f69ae3e910149c24f))

#### Bug Fixes

* **notify:**  bubble y position when dock in top mode ([c61ff4cd](https://github.com/linuxdeepin/dde-session-ui/commit/c61ff4cd66822a719e537117661b0bfab6756447))
* **osd:**  empty id of notify cache img ([97a06e15](https://github.com/linuxdeepin/dde-session-ui/commit/97a06e1537a8099e8c5d70657861c30228f81995))



<a name="4.4.8"></a>
### 4.4.8 (2018-07-31)


#### Bug Fixes

*   fix osd crash caused by notifications ([4e7c8dff](https://github.com/linuxdeepin/dde-session-ui/commit/4e7c8dff2a7d8fbdb9d3e091ae42e69e55d6dcfe))
*   bubble behind the control center ([0274f22d](https://github.com/linuxdeepin/dde-session-ui/commit/0274f22d8583875a680ff6711a63c424ba376e41))
* **dock:**  bubble position after suspend ([b971886f](https://github.com/linuxdeepin/dde-session-ui/commit/b971886f6c33a6cc184b037994bd2071f63d0c5f))
* **greeter:**
  *  Automatic scaling cannot be used in virtual machines ([70e9d3d6](https://github.com/linuxdeepin/dde-session-ui/commit/70e9d3d6b2d38adc47eef7627e6cb8351d5b495a))
  *  keyboard focus error ([bc200b98](https://github.com/linuxdeepin/dde-session-ui/commit/bc200b989c17bfb94678b190a114a930753fd4e9))
* **lock:**  keyboard grab ([36a775fb](https://github.com/linuxdeepin/dde-session-ui/commit/36a775fb8b4e6b40823dacbe935e0c470fee4b15))
* **notify:**  register bubble is dock type and keep bottom ([46a7c1dd](https://github.com/linuxdeepin/dde-session-ui/commit/46a7c1dda93dfb304339992fb00f744b332b996b))
* **osd:**  hide the container when is not operable ([ce944306](https://github.com/linuxdeepin/dde-session-ui/commit/ce944306d2e8c2342c5f275df3d267aee626b62b))

#### Features

* **greeter:**  update auto scale ([8563e5ab](https://github.com/linuxdeepin/dde-session-ui/commit/8563e5ab30cab111b8ac88ef1b2c72d2f028923c))



<a name="4.4.7"></a>
### 4.4.7 (2018-07-20)


#### Bug Fixes

* **osd:**  not show ([0cd7cc78](https://github.com/linuxdeepin/dde-session-ui/commit/0cd7cc78094775affcea33633b02930f42ba7dff))



<a name="4.4.6"></a>
### 4.4.6 (2018-07-20)


#### Performance

* **osd:**  merge deepin-notifications ([3bd9c2df](https://github.com/linuxdeepin/dde-session-ui/commit/3bd9c2dfbf7372aa50163ae33e944a172ca6541c))

#### Features

* **greeter:**  support auto scale ([c09a6862](https://github.com/linuxdeepin/dde-session-ui/commit/c09a6862ab81fbffd78540282fe6cd97b1703418))
* **osd:**  support replace old process with dbus ([5ed51ad3](https://github.com/linuxdeepin/dde-session-ui/commit/5ed51ad36544b794793b4b9cf4386532225d7d80))
* **welcome:**
  *  change single instance to user ([c3036448](https://github.com/linuxdeepin/dde-session-ui/commit/c30364486ae1e5846c36c908ff143f905c185105))
  *  support display system type ([15a79edb](https://github.com/linuxdeepin/dde-session-ui/commit/15a79edb66857dcb4032419458d40d154d11ef23))

#### Bug Fixes

*   invalid tip ui ([37bb45f9](https://github.com/linuxdeepin/dde-session-ui/commit/37bb45f98878b3bca6533d97d1cf1b91b83db6d8))
*   keyboard layout visible when show error tip ([9dee1308](https://github.com/linuxdeepin/dde-session-ui/commit/9dee130831042827eaf0117ccc8e80dd3f89963e))
* **background:**  multi screen rendering error ([68a74d44](https://github.com/linuxdeepin/dde-session-ui/commit/68a74d44eb8d89d3c72ed9c67d0c8877e61f386f))
* **dde-osd:**
  *  change the brightness just after starting up ([2a9fddcb](https://github.com/linuxdeepin/dde-session-ui/commit/2a9fddcb0c8a13f9112ff36f2d26a4c89580f6e4))
  *  change the volume just after starting up ([2cb43667](https://github.com/linuxdeepin/dde-session-ui/commit/2cb43667454aa3b3733de37bad58b9ba03eabbfe))
* **greeter:**  error hide keyboard widget ([0728e5f2](https://github.com/linuxdeepin/dde-session-ui/commit/0728e5f2e55a0fedd081b1d67b7fbc994c02b9cf))
* **osd:**
  *  null pointer exception notify ([cef0d1b3](https://github.com/linuxdeepin/dde-session-ui/commit/cef0d1b3ec03124f7d485d05f080c96fdfe51fc4))
  *  Calling dbus too early with notification ([9c9c80cf](https://github.com/linuxdeepin/dde-session-ui/commit/9c9c80cfac7cae784777d8efb33e8b4539573353))
  *  Calling dbus too early with keyboard dbus ([7b763a7f](https://github.com/linuxdeepin/dde-session-ui/commit/7b763a7f72f51c4ddb9b9fb99c5abfa29ac35ff4))
* **userwidget:**  not restore choose mode state ([9e3b0b34](https://github.com/linuxdeepin/dde-session-ui/commit/9e3b0b344d34d91e355f69d79e38751ee3a7f622))
* **welcome:**  not call show function ([befa9ffa](https://github.com/linuxdeepin/dde-session-ui/commit/befa9ffa6acaf64974728cceb371ea83e9f80c45))



<a name="4.4.5"></a>
### 4.4.5 (2018-06-13)


#### Bug Fixes

* **greeter:**  not clean lastuser file ([c49b2ab5](https://github.com/linuxdeepin/dde-session-ui/commit/c49b2ab59bdc3ee5187d592eb32a56e5d86d1bd4))



<a name="4.4.4"></a>
### 4.4.4 (2018-06-12)


#### Features

* **greeter:**  support virtual keyboard ([b1467c18](https://github.com/linuxdeepin/dde-session-ui/commit/b1467c18f86448d46fb6a74808fcf27002770c71))
* **lock:**  don't disable input method ([04037d1e](https://github.com/linuxdeepin/dde-session-ui/commit/04037d1e11a6c57ffea4543d9ce2c07b364317aa))

#### Bug Fixes

*   update failback background path ([4c092dfe](https://github.com/linuxdeepin/dde-session-ui/commit/4c092dfe6341da6df7ea108127a8e19f41857938))
* **background:**  null pixmap warning ([b31a01ba](https://github.com/linuxdeepin/dde-session-ui/commit/b31a01ba4316cd5b3581dcb96b202069194ffe47))
* **greeter:**  Listen to files to switch users ([fd5f5595](https://github.com/linuxdeepin/dde-session-ui/commit/fd5f5595776dc357fbe28aec58c64a8dd35ad154))
* **userwidget:**  not init islogind value ([e28454ae](https://github.com/linuxdeepin/dde-session-ui/commit/e28454ae4e48ccf72e39afc1652397c162710fcc))



<a name="4.4.3"></a>
### 4.4.3 (2018-06-07)


#### Bug Fixes

*   null pointer exception ([0eec4ec9](https://github.com/linuxdeepin/dde-session-ui/commit/0eec4ec9cc0861ee766da5b137b9dd6849bf33f5))
* **greeter:**  content change, unable to delete user ([4fa39ab1](https://github.com/linuxdeepin/dde-session-ui/commit/4fa39ab16c65d3f76c83969188e83dba4f83ce5d))
* **welcome:**  no tip when file not exist ([dc21d2fb](https://github.com/linuxdeepin/dde-session-ui/commit/dc21d2fb92ac6b785f0a8ea2b6c1526b5e3ed5a2))
* **wmchooser:**  not use failback wallpaper ([334e0e2f](https://github.com/linuxdeepin/dde-session-ui/commit/334e0e2f219ef8cce6e44922fe041e630afeee3e))



<a name="4.4.2"></a>
### 4.4.2 (2018-05-31)


#### Bug Fixes

*   remove old functions to fix build under Qt 5.11+ ([f8cf0703](https://github.com/linuxdeepin/dde-session-ui/commit/f8cf070316f7b197282f42fab83eb9951b00b0ea))
* **osd:**  display shelter when font size changed ([f0de859e](https://github.com/linuxdeepin/dde-session-ui/commit/f0de859edd5da9255668d4f069a9d0f7597f6366))
* **shutdown:**  warning label not wrap ([acc1fba6](https://github.com/linuxdeepin/dde-session-ui/commit/acc1fba6a513475ee206668709214c958a793c37))



<a name="4.4.1"></a>
### 4.4.1 (2018-05-29)


#### Bug Fixes

* **greeter:**  not update user keyboard layout ([a7b95034](https://github.com/linuxdeepin/dde-session-ui/commit/a7b95034d590b9afb75ce0586d3267d95e2c02eb))
* **lock:**  not hide keyboard widget ([037d27ea](https://github.com/linuxdeepin/dde-session-ui/commit/037d27ea14f24403085debe5f083135ac3d05bb0))
* **shutdown:**  content is cut off cause high limit ([859a32d0](https://github.com/linuxdeepin/dde-session-ui/commit/859a32d02edee66ab044d14cb756494a8de65b6e))



<a name=""></a>
##  4.4.0 (2018-05-24)


#### Features

*   scroll user widget ([cea5cde4](https://github.com/linuxdeepin/dde-session-ui/commit/cea5cde4795f0252e2065eea7c2ed598049b7611))
*   disable login animation for mips ([f34524a9](https://github.com/linuxdeepin/dde-session-ui/commit/f34524a91b1226e11334fcc9b000cf13df5e37e0))
*   Limit proc min memory usage is display to 10MB ([9efdc37b](https://github.com/linuxdeepin/dde-session-ui/commit/9efdc37bed50630c5094a72d3cf7886739f693e9))
* **osd:**
  *  auto exit for dde-osd ([cd762f62](https://github.com/linuxdeepin/dde-session-ui/commit/cd762f6225415642c37847fc2e6379be02373a99))
  *  support 150 volume ([a7ce922a](https://github.com/linuxdeepin/dde-session-ui/commit/a7ce922ae980d1365ea951942b714b120ab41de1))

#### Bug Fixes

*   force kill application ([fdfff7a0](https://github.com/linuxdeepin/dde-session-ui/commit/fdfff7a078a1c54a538debf3be63961fad405725))
*   not avatar when disable animation ([57d836cc](https://github.com/linuxdeepin/dde-session-ui/commit/57d836ccefff5716fb10f63556cfefff2e0b2823))
*   Compile error because of macro ([2509fcd2](https://github.com/linuxdeepin/dde-session-ui/commit/2509fcd25c60e3017f00f401ef1c2bebf4e76850))
*   not clean user password ([ac9c7836](https://github.com/linuxdeepin/dde-session-ui/commit/ac9c78363f3c4d538e70b067aa7c2e5084f14da0))
* **UserWidget:**  error height ([0598602c](https://github.com/linuxdeepin/dde-session-ui/commit/0598602c0a8ce8bb6a58ebb6d335d9962562d611))
* **dmemory:**  regular expression capture error ([72c9b6a0](https://github.com/linuxdeepin/dde-session-ui/commit/72c9b6a042a2e5e3fbb9c70d86e32677e1c33578))
* **lock:**  disable DMPRIS widget picture ([e9e34770](https://github.com/linuxdeepin/dde-session-ui/commit/e9e34770e88dbae83fc9113e285da672d4bc1212))
* **memory-warning-dialog:**  deepin-wm is in the free list ([3e2a3aac](https://github.com/linuxdeepin/dde-session-ui/commit/3e2a3aace3a3f5d3acc369831762ef3f093c1a8a))
* **shutdown:**  switch user button will show when prompt interface ([ebf65469](https://github.com/linuxdeepin/dde-session-ui/commit/ebf6546974c5665856febb728c0ca76c25efe7b7))
* **welcome:**  small version of the upgrade will also be prompted ([102b048a](https://github.com/linuxdeepin/dde-session-ui/commit/102b048af70fc7c5e68d742039877281a53b03ee))
* **wm-chooser:**  json type error ([8b1df153](https://github.com/linuxdeepin/dde-session-ui/commit/8b1df153b7ae3fb9eb22aa5d76f5e3750985f882))



<a name="4.3.7"></a>
### 4.3.7 (2018-03-29)


#### Bug Fixes

* **greeter:**  not check user state ([543c79b6](https://github.com/linuxdeepin/dde-session-ui/commit/543c79b618c0fb5e5b2d61e5ee88791b8f2fc906))



<a name="4.3.6"></a>
### 4.3.6 (2018-03-28)


#### Bug Fixes

*   AD user login prompt error ([4f7687c8](https://github.com/linuxdeepin/dde-session-ui/commit/4f7687c8c7bc85c90a91bd60ab3c4d6c771d75d1))



<a name="4.3.5"></a>
### 4.3.5 (2018-03-27)




<a name="4.3.4"></a>
### 4.3.4 (2018-03-27)


#### Bug Fixes

*   remove the blank at the begging and end of the message ([9f0a84a7](https://github.com/linuxdeepin/dde-session-ui/commit/9f0a84a731499343116abeea3ee5a4f2c179ba26))
* **greeter:**  esc abort operation not restore widget visible ([f8c7b79d](https://github.com/linuxdeepin/dde-session-ui/commit/f8c7b79d041b9d22693e4003f55bcb2c9aece3b8))



<a name="4.3.3"></a>
### 4.3.3 (2018-03-26)


#### Bug Fixes

* **greeter:**  not need check current user ([56c13603](https://github.com/linuxdeepin/dde-session-ui/commit/56c13603848fae82a2fc162d98d6f0d788e904f2))



<a name="4.3.2"></a>
### 4.3.2 (2018-03-22)


#### Features

*   If the last user was not found, initialize the AD login button ([dd5cf0be](https://github.com/linuxdeepin/dde-session-ui/commit/dd5cf0be55638da8d4732b33faf96d56f88e3aa2))
*   keep user list order ([589c58e1](https://github.com/linuxdeepin/dde-session-ui/commit/589c58e1554f3a9f1fd6f2097f9dcc2629fd377e))

#### Bug Fixes

*   alert tip not hide ([3efa4cf7](https://github.com/linuxdeepin/dde-session-ui/commit/3efa4cf74f56dfa23ff44818e85369b570d023ae))
*   not emit user list changed when append user ([3f70d9a8](https://github.com/linuxdeepin/dde-session-ui/commit/3f70d9a83945721b62c9dd793c3e0a9de16dc317))
* **fullscreen:**  Handle multi-screen geometry correctly ([a2dede2b](https://github.com/linuxdeepin/dde-session-ui/commit/a2dede2b1418964a00a521786ae2061076d3a83a))
* **greeter:**
  *  not select user kblayout ([6b04a65d](https://github.com/linuxdeepin/dde-session-ui/commit/6b04a65dae86bbf59fa8617ec160f34e3612b9e9))
  *  not set greeter authenticate user ([a50c9a78](https://github.com/linuxdeepin/dde-session-ui/commit/a50c9a78edc4f69df681e843f30cb523eb941f62))
  *  Prevent duplicate settings of current user ([c1ab1484](https://github.com/linuxdeepin/dde-session-ui/commit/c1ab1484818011ab76bbe383101b25d33e923ef4))
* **osd:**  not fillet after startup ([aec60e2e](https://github.com/linuxdeepin/dde-session-ui/commit/aec60e2e7e9970ad5af204cd4f945db86465dda3))
* **welcome:**
  *  restore hotzone with welcome quit ([307f1de4](https://github.com/linuxdeepin/dde-session-ui/commit/307f1de41ce2007e39f222849d92f964ac857735))
  *  error dbus call ([d21d766e](https://github.com/linuxdeepin/dde-session-ui/commit/d21d766ebe561d3619d8f7bb244e9e8728f9bd56))



<a name="4.3.1"></a>
### 4.3.1 (2018-03-16)


#### Bug Fixes

*   dde warning dialog not move to center ([f8753511](https://github.com/linuxdeepin/dde-session-ui/commit/f8753511fcecd2b972a8e56a2b10f3849deccd47))
*   check cursor position error when screens scale ([94b4e4c1](https://github.com/linuxdeepin/dde-session-ui/commit/94b4e4c1a0fcb5b6ed6cc9773aa5fc79993d2c15))
*   black screen when not have blur image ([1b049645](https://github.com/linuxdeepin/dde-session-ui/commit/1b049645e2833270097df3b8fa369c28f38ec237))
* **greeter:**  refresh the AD state after staring ([09bd9d69](https://github.com/linuxdeepin/dde-session-ui/commit/09bd9d69e869bcb1236d5c4ad0dfbd6572637072))
* **lock:**  error return greeter wallpaper ([f34bf490](https://github.com/linuxdeepin/dde-session-ui/commit/f34bf490bc9465a94d53c3e48398f05aeea45d5d))
* **welcome:**  not disable zone ([0cacd0a9](https://github.com/linuxdeepin/dde-session-ui/commit/0cacd0a979c19554b432c72003bf2f4f4fd54083))
* **widgets:**  user icon not align center and icon size error ([0c2b4f64](https://github.com/linuxdeepin/dde-session-ui/commit/0c2b4f6482cae797c753d54b67ee5b8609f63ec5))



<a name=""></a>
##  4.3.0 (2018-03-14)


#### Features

*   remove dman portal, use deepin-manual. ([3bd028cf](https://github.com/linuxdeepin/dde-session-ui/commit/3bd028cf135ad22c784c0146e447ef34a69af768))
*   add runtime dep ([f4a86d1b](https://github.com/linuxdeepin/dde-session-ui/commit/f4a86d1b9cf82073b0492a67d70a5bd2a7523d7b))
* **dde-welcome:**  Set the mouse style to wait ([b421fc33](https://github.com/linuxdeepin/dde-session-ui/commit/b421fc33a25f7e111deb722cc4c2f0993d0add6b))
* **greeter:**
  *  save last user ([7ffb768e](https://github.com/linuxdeepin/dde-session-ui/commit/7ffb768ef0654d021e5e6fe73c9159584552a64e))
  *  support other login ([1466f7fd](https://github.com/linuxdeepin/dde-session-ui/commit/1466f7fd95636254b09b1791a44980048fa7f3fe))
  *  support AD user login ([c70c613a](https://github.com/linuxdeepin/dde-session-ui/commit/c70c613ab1ddc683de09f945dfdfa2a523b9c077))
* **lock:**
  *  support switch to other ([e8a8b438](https://github.com/linuxdeepin/dde-session-ui/commit/e8a8b438172b391e6c16a82ab9ce07f4e5ba0685))
  *  support other login ([8c2d7277](https://github.com/linuxdeepin/dde-session-ui/commit/8c2d72770cd9bb8b5a3948e7d71b485e43550404))
* **osd:**  use new dock indicator plugin config ([f34edcc7](https://github.com/linuxdeepin/dde-session-ui/commit/f34edcc76eb7a2261b5189fdd319f28a07922a75))
* **otheruserinput:**
  *  add alert tip ([d37d7401](https://github.com/linuxdeepin/dde-session-ui/commit/d37d74014ad7b2a7cefe88d5526043c609c8aadf))
  *  add new widget ([eda373ab](https://github.com/linuxdeepin/dde-session-ui/commit/eda373abffb689acd457e55ecdafba6d43d79559))
* **userwidget:**  use signal update button ([bc275b31](https://github.com/linuxdeepin/dde-session-ui/commit/bc275b31c2b920671484949e8f585b5ecbadb971))
* **welcome:**  support login animation ([8d614a71](https://github.com/linuxdeepin/dde-session-ui/commit/8d614a7189dfe7eb63300cb3a4be4fb62c7db48b))

#### Bug Fixes

*   switch to user not grab keyboard ([2b594f1b](https://github.com/linuxdeepin/dde-session-ui/commit/2b594f1b5a67dbed70a657b3de34ac7800d5dd92))
*   click user icon will refresh password edit ([d19f5831](https://github.com/linuxdeepin/dde-session-ui/commit/d19f583111cbbea557a7adbd5a83bb64018c9ff2))
*   using new dbus interface ([2df0514e](https://github.com/linuxdeepin/dde-session-ui/commit/2df0514e882c1263e57d73bd960a72d0404f0a30))
*   widgets position error ([18b2d19e](https://github.com/linuxdeepin/dde-session-ui/commit/18b2d19e9182a96cb5a7e1644f1231924a45bf2b))
*   session cannot choose one session of enter key ([55e8eda0](https://github.com/linuxdeepin/dde-session-ui/commit/55e8eda0905aee6315d76133fd04a54ebcf642c3))
*   not update user widget when choosed session ([262b020f](https://github.com/linuxdeepin/dde-session-ui/commit/262b020f7c6a46711e2eb7235a2708607bc6a99e))
*   fullscreen background geometry error ([d5fa74c6](https://github.com/linuxdeepin/dde-session-ui/commit/d5fa74c6d23afd119b4ec787a4a2b751385a7ab0))
*   set focus when program startup ([6fe04c6a](https://github.com/linuxdeepin/dde-session-ui/commit/6fe04c6a2f8781e581c0088ef069bd933742ac87))
*   Adapt lintian ([b32cdb0c](https://github.com/linuxdeepin/dde-session-ui/commit/b32cdb0c3538b3c285d79d4409012430e100f442))
* **UserWidget:**  update buttons position ([11c5eb1d](https://github.com/linuxdeepin/dde-session-ui/commit/11c5eb1d2e763d06cacb60295f550f03baeab68a))
* **background:**  set default wallpaper when file not exist ([ec513677](https://github.com/linuxdeepin/dde-session-ui/commit/ec5136778cd497b1ad63356872e2accf4e3ad156))
* **dde-welcome:**  not save log to .cache/deepin ([430e99d4](https://github.com/linuxdeepin/dde-session-ui/commit/430e99d4be4bbfa6d8a809a8167e6f1403eea8ab))
* **greeter:**
  *  widgets position error ([2d829efc](https://github.com/linuxdeepin/dde-session-ui/commit/2d829efcabc65e4f301d381d481e026efc4fe093))
  *  widgets position error ([18904382](https://github.com/linuxdeepin/dde-session-ui/commit/18904382e6c38446a0ba223c843c937e646d43bb))
  *  not hide userwidget when choose session and power ([e15195c2](https://github.com/linuxdeepin/dde-session-ui/commit/e15195c29683e370a60940f2d486b4fb9cdb016d))
  *  not auth user ([9ffe43c8](https://github.com/linuxdeepin/dde-session-ui/commit/9ffe43c89d8a9f6c96ae3aaa502957f00e0dc82e))
  *  nopasswd user cannot login ([a6fd7ab4](https://github.com/linuxdeepin/dde-session-ui/commit/a6fd7ab4fcbde865b6b841afbd2577e1dea75faf))
  *  add timeout ([91bf150d](https://github.com/linuxdeepin/dde-session-ui/commit/91bf150d19fdaca93546e7c8b8d6b73107869d42))
  *  crash when can't find session name ([9efab64e](https://github.com/linuxdeepin/dde-session-ui/commit/9efab64e69ebba804c0ab84d25bc77b04cca9648))
  *  use new background dbus ([a5157308](https://github.com/linuxdeepin/dde-session-ui/commit/a515730865b5b7b99251e1579249815b74a8e505))
  *  cursor not apply scale ([943ae919](https://github.com/linuxdeepin/dde-session-ui/commit/943ae9192b92acd78bee3145972fad639d520d4d))
  *  remove save last session ([dcf12214](https://github.com/linuxdeepin/dde-session-ui/commit/dcf12214519001b9ed79005016a898845becf9a0))
* **layout:**  fix user name layout ([3d68229c](https://github.com/linuxdeepin/dde-session-ui/commit/3d68229ca0da14061fc8cd775edadc8ba23cd672))
* **lock:**
  *  not grab keyboard ([3b20c2c3](https://github.com/linuxdeepin/dde-session-ui/commit/3b20c2c3a0dfb9f2ccb907d6274a14c48a017be0))
  *  canel shutdown not hide warning tip ([98841fc0](https://github.com/linuxdeepin/dde-session-ui/commit/98841fc096c74aae57261a520df930133733eedb))
  *  control center not hide ([c465e367](https://github.com/linuxdeepin/dde-session-ui/commit/c465e367aec8b80b9a1ebbb8346ae5fa0a883d51))
  *  No focus after staring ([8ef1f837](https://github.com/linuxdeepin/dde-session-ui/commit/8ef1f837c578653e86626c9392db0bd88387cc20))
  *  volume number color is black ([575a8a48](https://github.com/linuxdeepin/dde-session-ui/commit/575a8a48dc8b96ead2cf0b3a714f005eb75e1ca2))
  *  mpris widget not set style ([a587d21a](https://github.com/linuxdeepin/dde-session-ui/commit/a587d21a0cc3aea22565edfbdb82ad2a286061cd))
* **shutdown:**
  *  not check wallpaper is local file ([57839e13](https://github.com/linuxdeepin/dde-session-ui/commit/57839e139da7617721b3b5f371795b78e36161d8))
  *  remove multi user warning label's fixed width ([223e9fac](https://github.com/linuxdeepin/dde-session-ui/commit/223e9fac5427244165ea4262c5bfa25d4b17134f))
* **welcome:**
  *  update success svg not support HiDPI ([7cbe1b22](https://github.com/linuxdeepin/dde-session-ui/commit/7cbe1b2285575fcd04252e056f81d01fdb80cc4c))
  *  Set the wrong window flag ([05458296](https://github.com/linuxdeepin/dde-session-ui/commit/05458296ff020a2a81c40ac78e059b4c3ed41ced))
  *  fit multi screen ([09c211cb](https://github.com/linuxdeepin/dde-session-ui/commit/09c211cb5cfc26a87ce2da401a0f5cc656eb73c6))
* **widgets:**
  *  use compatibility interface with user background ([6375fde4](https://github.com/linuxdeepin/dde-session-ui/commit/6375fde445ea1bd57ba152ba647e7e0a9949232c))
  *  AD user cannot login when userwidget not releasekeyboard ([5c1043ea](https://github.com/linuxdeepin/dde-session-ui/commit/5c1043ead4626d3933aed2d74e062ef7cf7adef4))
  *  setting properties for many times ([c3ba458b](https://github.com/linuxdeepin/dde-session-ui/commit/c3ba458bd45bdb7d0c107f55ff91e616318de0d5))
  *  other user input set focus type ([03912aa5](https://github.com/linuxdeepin/dde-session-ui/commit/03912aa5ddf105297d6095261ebf9c42f7e18ffe))



<a name="4.1.7"></a>
### 4.1.7 (2017-11-28)


#### Bug Fixes

* **greeter:**
  *  unable to login other desktop when user is nopassword ([4827e09b](https://github.com/linuxdeepin/dde-session-ui/commit/4827e09b1f3b7e9ab3efdfd9ab4ab5f99892c84a))
  *  error auth password when nopassword user ([e3986aa8](https://github.com/linuxdeepin/dde-session-ui/commit/e3986aa8123410b84c49dbf2b6ebd90d9f0225c8))



<a name="4.1.6"></a>
### 4.1.6 (2017-11-23)


#### Bug Fixes

* **lock:**  don't show alert when user is nopasswdlogin ([97f52db0](https://github.com/linuxdeepin/dde-session-ui/commit/97f52db085b285030cdc381c9c7782f72a5cdf7b))
* **shutdown:**  cannot open system monitor ([7e3156fa](https://github.com/linuxdeepin/dde-session-ui/commit/7e3156fa6e35d1710938e5af0fcb2b9678c20b09))



<a name="4.1.5"></a>
### 4.1.5 (2017-11-20)


#### Features

* **osd:**  change hide time to 1 second ([615d7208](https://github.com/linuxdeepin/dde-session-ui/commit/615d7208c864c84454c4dbe1e2e449e45f0fae5a))



<a name="4.1.4"></a>
### 4.1.4 (2017-11-16)


#### Bug Fixes

* **shutdown:**  background white ([d1f8809c](https://github.com/linuxdeepin/dde-session-ui/commit/d1f8809c0ed85956ce79c963f0ddf1cdf64baff2))



<a name="4.1.3"></a>
### 4.1.3 (2017-11-16)


#### Bug Fixes

* **background:**  refresh wallpaper when not the same file ([43931726](https://github.com/linuxdeepin/dde-session-ui/commit/43931726241f5e42a8f7b07cc88e435a11a30e07))
* **dde-lock:**  some widget shown too early ([fa857876](https://github.com/linuxdeepin/dde-session-ui/commit/fa85787610cc18765bf19cec0230171b2554a332))
* **greeter:**  skip when click self ([3128a549](https://github.com/linuxdeepin/dde-session-ui/commit/3128a54987df85c6c9e3407d1be2c55008506cc1))
* **lock:**  userwidget pos error when size changed ([13775f5b](https://github.com/linuxdeepin/dde-session-ui/commit/13775f5b66f0faea0dd58ed3a11e688e69214373))

#### Features

*   add dde-dman-portal ([f7772551](https://github.com/linuxdeepin/dde-session-ui/commit/f77725511b03162073699ca8c67ee9292f6d8363))
* **shutdown:**  support display user displayname ([c7a3fe69](https://github.com/linuxdeepin/dde-session-ui/commit/c7a3fe69780e1f9c209e5ea5044fabc43d948545))
* **user:**  support display user fullname ([34f2f9ff](https://github.com/linuxdeepin/dde-session-ui/commit/34f2f9ffc48819ec46d4b62f76e48452cf4b8d0e))
* **userwidget:**  login user interface updated ([7eeeb589](https://github.com/linuxdeepin/dde-session-ui/commit/7eeeb58991a7fbe76e391a9fd046c57f3c35a635))



<a name="4.1.2"></a>
### 4.1.2 (2017-11-10)


#### Bug Fixes

* **greeter:**  nopasswordlogin function failure ([c02c4d3c](https://github.com/linuxdeepin/dde-session-ui/commit/c02c4d3cd9eb6c7f4a2a337542ed8eee436b0a75))
* **lock:**  use dgettext update ts with fprint ([43ba4a3e](https://github.com/linuxdeepin/dde-session-ui/commit/43ba4a3e6369e7db57bba1f04df94c8690d27bbb))
* **shutdown:**  set default background when get blur error ([9f7ab4e2](https://github.com/linuxdeepin/dde-session-ui/commit/9f7ab4e2caa567275e892477e6667feeac46828f))



<a name="4.1.1"></a>
### 4.1.1 (2017-11-09)


#### Bug Fixes

*   osd support multiple screens list ([5a8b880c](https://github.com/linuxdeepin/dde-session-ui/commit/5a8b880cc7988c4774f59bab1888304b128f2ca7))
* **greeter:**  switch user will login ([4cb700d7](https://github.com/linuxdeepin/dde-session-ui/commit/4cb700d75cb9fae8ec30936b97bffdf68338110a))
* **hidpi:**  window placed at wrong position ([8252e1ee](https://github.com/linuxdeepin/dde-session-ui/commit/8252e1ee5f2015cec76f28430a64fc0f8fba665b))
* **lock:**  update ts with fprint ([acfa282c](https://github.com/linuxdeepin/dde-session-ui/commit/acfa282c362a96ec4788de34d279ac9951fcbf7e))
* **osd:**
  *  support dapplication single instance ([d04ef2e8](https://github.com/linuxdeepin/dde-session-ui/commit/d04ef2e858a5c9744e14d93d244c993cafe64a25))
  *  scroll to current item when direct switch ([766a1116](https://github.com/linuxdeepin/dde-session-ui/commit/766a11161f9b2bcd9530f436427c70771f47dae7))

#### Features

* **background:**  use blur image dbus ([2f135276](https://github.com/linuxdeepin/dde-session-ui/commit/2f135276126bd9ad48a6ad446a48fc3190d98930))
* **dde-welcome:**  adding detection rules ([426bc903](https://github.com/linuxdeepin/dde-session-ui/commit/426bc903ac40093cbc5961f1843baa81fbb17e1e))



<a name=""></a>
##  4.1.0 (2017-11-03)


#### Features

* **dde-warning-dialog:**  dbus interface ([4bd30af4](https://github.com/linuxdeepin/dde-session-ui/commit/4bd30af46ad76d09257ed319a0f3f68945945430))
* **shutdown:**
  *  use dapplication and set default theme ([1546dd20](https://github.com/linuxdeepin/dde-session-ui/commit/1546dd2007a092e66141301fca3957ac3457d7d9))
  *  apply system font size settings ([65b8a11f](https://github.com/linuxdeepin/dde-session-ui/commit/65b8a11f9b16e53c9a264239f2d8624e70cb468f))

#### Bug Fixes

* **lock:**
  *  not show error message when finger verification ([e831d6bd](https://github.com/linuxdeepin/dde-session-ui/commit/e831d6bd8c1debbbbfca4b53b033593a8c733975))
  *  show wrong passwd premature ([0a56de2e](https://github.com/linuxdeepin/dde-session-ui/commit/0a56de2e6d259ab4bfd2233ea56c9e81043d15aa))
* **osd:**  cannot scroll when error index ([6f307bc1](https://github.com/linuxdeepin/dde-session-ui/commit/6f307bc1584199d7ff2824ba6d9e347285c27b45))
* **shutdown:**
  *  not grab keyboard ([aac6cf88](https://github.com/linuxdeepin/dde-session-ui/commit/aac6cf88ed84a49c5a3f398f35ed5383eee2f5d5))
  *  white screen when not use deepin wm ([9a5c78db](https://github.com/linuxdeepin/dde-session-ui/commit/9a5c78db5d0c4e3f6baa79266eb10258f9f84078))
  *  not disable hotzone when show ([539c8b99](https://github.com/linuxdeepin/dde-session-ui/commit/539c8b997f735fd5ff794463d2554c945002b1e1))



<a name=""></a>
##  4.0.17 (2017-10-26)


#### Bug Fixes

*   svg file error ([684de9e7](https://github.com/linuxdeepin/dde-session-ui/commit/684de9e76cd66eef384045d5671c3a5dbe907177))
*   correct header filename ([7cc8316b](https://github.com/linuxdeepin/dde-session-ui/commit/7cc8316bf62617e4e6fb0093e8fa5fa6f30c14fb))
*   upgrade success not load ts ([a709cc88](https://github.com/linuxdeepin/dde-session-ui/commit/a709cc88bb2c12034b1574756209a3a661f273ee))
*   remove libpam0g from debian/control ([2b3171de](https://github.com/linuxdeepin/dde-session-ui/commit/2b3171deddea292e4c081bc701a15fe9e080a1bc))



<a name=""></a>
##  (2017-10-25)


#### Bug Fixes

*   shutdown contentwidget not init ([ee42e21d](https://github.com/linuxdeepin/dde-session-ui/commit/ee42e21d9b60bab01af389e0e492ce9bba255808))

#### Features

*   add finger verification timed out tip ([4640a828](https://github.com/linuxdeepin/dde-session-ui/commit/4640a828a32dafba79e333065562063d32007bd7))



