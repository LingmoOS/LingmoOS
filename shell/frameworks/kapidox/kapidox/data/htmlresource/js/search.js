var entityMap = {
  '&': '&amp;',
  '<': '&lt;',
  '>': '&gt;',
  '"': '&quot;',
  "'": '&#39;',
  '/': '&#x2F;',
  '`': '&#x60;',
  '=': '&#x3D;'
};

function escapeHtml (string) {
  return String(string).replace(/[&<>"'`=\/]/g, function (s) {
    return entityMap[s];
  });
}

function GetURLParameter(sParam)
{
    var sPageURL = window.location.search.substring(1);

    var sURLVariables = sPageURL.split('&');
    for (var i = 0; i < sURLVariables.length; i++) {
        var sParameterName = sURLVariables[i].split('=');
        if (sParameterName[0] == sParam) {
            return escapeHtml(decodeURIComponent(sParameterName[1]));
        }
    }
    return ""
}

async function render_search(type)
{
    var query = GetURLParameter("query");
    if (query == "") {
        $( '.loader' ).remove()
        $( "#search-title" ).html("<i>...If you don't tell what to search, I can't find anything...</i>");
        return
    }
    $( "#search-input" ).val(query);
    $( "#search-title" ).append(" <i>" + query + "</i>");
    var json_path =  "searchdata.json";

    await do_search(json_path, query, type);
}

function do_search(json_path, query, type)
{
    var results = []
    $.ajax({
        url: json_path,
        dataType: 'json',
        error: function(data, status, err) {
            console.log(err)
        },
        success: function (json) {
            search_json(type, json, query)
        }
    });
}

function search_json(type, json, query)
{
    result_html = ""
    if (type == 'library') {
        results_html = search_json_library(json, query)
    } else if (type == 'group') {
        results_html = search_json_group(json, query)
    } else if (type == 'global') {
        results_html = search_json_global(json, query)
    }

    $( '.loader' ).remove()
    $( '#results' ).append(results_html)

    document.querySelectorAll('h1, .dynheader, el, #results li a').forEach(h => {
      h.innerText = h.innerText.replace('org::kde::kirigami::templates::', 'Kirigami.Templates.');
      h.innerText = h.innerText.replace('org::kde::kirigami::', 'Kirigami.');
    });

}

function search_json_library(json, query)
{
    var results_name = []
    var results_text = []
    $.each(json.docfields, function(key, val) {
        if ('name' in val) {
            if (val.name.search(new RegExp(query, "i")) != -1) {
                results_name.push(val)
            }
        }
        if ('text' in val) {
            if (val.text.search(new RegExp(query, "i")) != -1) {
                results_text.push(val)
            }
        }
    });

    var html_results = ""
    html_results += "<h3>Matches in names</h3>\n"
    html_results += "<ul>\n"
    $.each(results_name, function(key, result) {
        html_results += "\t<li><a href=\"" + result.url + "\">"+ result.name + "</a>\n"
    });
    html_results += "</ul>\n"

    html_results += "<h3>Matches in text</h3>\n"
    html_results += "<ul>\n"
    $.each(results_text, function(key, result) {
        html_results += "\t<li><a href=\"" + result.url + "\">"+ result.name + "</a>: " + result.text +"\n"
    });
    html_results += "</ul>\n"

    return html_results
}

function search_json_group(json, query)
{
    var results_name = []
    var results_text = []

    $.each(json.libraries, function(k, libval) {
        if ('fancyname' in libval) {
            libname = libval.fancyname
        }

        $.each(libval.docfields, function(key, val) {
            if ('name' in val) {
                if (val.name.search(new RegExp(query, "i")) != -1) {
                    val['libname'] = libname
                    results_name.push(val)
                }
            }
            if ('text' in val) {
                if (val.text.search(new RegExp(query, "i")) != -1) {
                    val['libname'] = libname
                    results_text.push(val)
                }
            }
        });
    });

    var html_results = ""
    html_results += "<h3>Matches in names</h3>\n"
    html_results += "<ul>\n"
    $.each(results_name, function(key, result) {
        html_results += "\t<li><a href=\"" + result.url + "\">"+ result.name + "</a> in <i>" + result.libname +"</i>\n"
    });
    html_results += "</ul>\n"

    html_results += "<h3>Matches in text</h3>\n"
    html_results += "<ul>\n"
    $.each(results_text, function(key, result) {
        html_results += "\t<li><a href=\"" + result.url + "\">"+ result.name + "</a> in <i>" + result.libname +"</i><br />" + result.text +"\n"
    });
    html_results += "</ul>\n"

    return html_results
}

function search_json_global(json, query)
{
    var results_name = []
    var results_text = []

    $.each(json.all, function(k, productval) {
        if ('fancyname' in productval) {
            productname = productval.fancyname
        }

        $.each(productval.libraries, function(k, libval) {
            if ('fancyname' in libval) {
                libname = libval.fancyname
            }

            $.each(libval.docfields, function(key, val) {
                if ('name' in val) {
                    if (val.name.search(new RegExp(query, "i")) != -1) {
                        val['libname'] = libname
                        val['productname'] = productname
                        results_name.push(val)
                    }
                }
                if ('text' in val) {
                    if (val.text.search(new RegExp(query, "i")) != -1) {
                        val['libname'] = libname
                        val['productname'] = productname
                        results_text.push(val)
                    }
                }
            });
        });
    });

    var html_results = ""
    html_results += "<h3>Matches in names</h3>\n"
    html_results += "<ul>\n"
    $.each(results_name, function(key, result) {
        html_results += "\t<li><a href=\"" + result.url + "\">"+ result.name +
            "</a> in <i>" + result.libname +"</i> from product <i>" +
            result.productname + "</i>\n"
    });
    html_results += "</ul>\n"

    html_results += "<h3>Matches in text</h3>\n"
    html_results += "<ul>\n"
    $.each(results_text, function(key, result) {
        html_results += "\t<li><a href=\"" + result.url + "\">"+ result.name +
            "</a> in <i>" + result.libname +"</i> from product <i>" +
            result.productname + "</i><br />" + result.text +"\n"
    });
    html_results += "</ul>\n"

    return html_results
}
