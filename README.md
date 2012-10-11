xbmc-txupdate
=============

This utility is suitable for keeping XBMC upstream language files and the language files hosted on transifex.net in sync.

What it does:
* Downloads the fresh files from upstream http URLs specified in the xbmc-txupdate.xml file and also downloads the fresh translations from transifex.net and makes a merge of the files. 
* With this merge process it creates fresh files containing all changes upstream and on transifex. 
* These updated files than can be commited to upstream repositories for end user usage.
* During the merge process it also creates update files which only contain the new upstream version if the English language files and also contain the new English strings translations introduced for different languagees on the upstream repository. These update PO files than can automatically uploaded to transifex with this utility.

Important to note that in case we both have a translation at the upstream repository and we have a translation at transifex.net for the same English string, the utility prefers the one at transifex.net. This means that new translations modifications can only be pulled from ustream into the merged files in case they are for completely newly introduced English strings which do not have translation existing at transifex yet.

Requirements:
OS: Linux
Packages: curl, libcurl, libjsoncpp

## Usage:


  **xbmc-txpudate PROJECTDIR [working mode]**


  * **PROJECTDIR:** the directory which contains the xbmc-txupdate.xml settings file and the .passwords file. This will be the directory where your merged and transifex update files get generated.
  * **Working modes:**
    * **-d**    Only download to local cache, without performing a merge.
    * **-dm**    Download and create merged and tx update files, but no upload performed.
    * **-dmu**    Download, merge and upload the new files to transifex.
    * **-u**    Only upload the previously prepared files. Note that this needs downlad and merge ran before.
    * No working mode arguments used, performs as -dm

Please note that the utility needs at least a projet setup xml file in the PROJECTDIR to work. Additionally it needs to have a passwords xml file which holds credentials for Transifex.com and for the http places you are using as upstream repositories.

## Setting files
In you PROJECTDIR folder you need to have the following files:
* **xbmc-txupdate.xml**
The format of the file looks like this:

```xml
<?xml version="1.0" ?>
<resources projectname="">
      <resource name="">
        <TXname></TXname>
        <upstreamURL></upstreamURL>
        <upstreamLangs></upstreamLangs>
        <resourceType></resourceType>
        <resourceSubdir></resourceSubdir>
    </resource>
</resources>
```
Where:
  * projectname: The Transifex projectname. Use the exact same string as on Transifex.
    Optional attributes:
      * http_cache_expire - (default: 360) expirity time for cached files in minutes. If cache file is younger than the given time, no actual http download will happen. In that case the cached file gets used.
      * min_completion - (default: 10%) a limit for the translated percentage to actually download a translation file.
      * merged_langfiledir - (default: merged-langfiles) the directory under PROJECTDIR, where the fresh merged, cleaned translations will be locally created.
      * temptxupdate_langfiledir - (default: tempfiles_txupdate) the directory under PROJECTDIR, where the language files to update Transifex will be locally created.
  * name: The name of the resource(plugin). This is the name which will be used as a directory name where the language files will be created. So the best is to use the same directory name here which is used at the upstream repo. This field must be filled.
  * TXname: The Transifex name you want to have for the resource(plugin). No special characters alowed(nor dot). This field must be filled.

