function get_files
{
    echo kde6.xml
}

function po_for_file
{
    case "$1" in
       kde6.xml)
           echo kde6_xml_mimetypes.po
       ;;
    esac
}

function tags_for_file
{
    case "$1" in
       kde6.xml)
           echo comment
       ;;
    esac
}

