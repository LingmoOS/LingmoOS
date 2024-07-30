function get_files
{
    echo bluedevil-mime.xml
}

function po_for_file
{
    case "$1" in
       bluedevil-mime.xml)
           echo bluedevil_xml_mimetypes.po
       ;;
    esac
}

function tags_for_file
{
    case "$1" in
       bluedevil-mime.xml)
           echo comment
       ;;
    esac
}
