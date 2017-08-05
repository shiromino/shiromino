
// override function of same name in common.js
// if gui editor possible, display the link to switch between text and gui editor
function show_switch2gui() {
    // Show switch to gui editor link if the browser is compatible
    if (can_use_gui_editor() === false) {return;}
    // instead of switch2gui, get xswitch2gui
    var switch2gui = document.getElementById('xswitch2gui');
    if (switch2gui) {
        switch2gui.style.display = 'block';
    }
}

// if user clicks on sidepanel link, click hidden buttons in editor form
// get corresponding element without leading x in its name and click it
function tickle(x){
    var yName;
    if (x.name) {
        yName = x.name.slice(1);
    } else {
        yName = x.parentNode.name.slice(1);
    }
    var y = document.getElementsByName(yName);
    y = y[0];
    y.click();
    return false;
}

// if no comment was entered for edit log, remind user
function checkComment(x) {
    var editcomment = document.getElementById('editor-comment');
    var newComment;
    if (editcomment.value === "") {
        newComment = prompt("Enter optional comment for change log:","");
        if (newComment === null) {
            // user hit cancel, cancel save
            return false;
        }
        editcomment.value = newComment;
    }
    // click submit button again
    tickle(x);
    return true;
}

// if there is a draft version of page, show button
function check_loaddraft() {
    var el = document.getElementsByName('button_load_draft');
    if (el[0]) {
        el = document.getElementsByName('xbutton_load_draft');
        if (el[0]) {
            el[0].parentNode.style.display = '';
        }
    }
}
jQuery(check_loaddraft);

// make all sidepanel links pointing to current page bold via CSS
// find all links pointing to this page and add classname 'thispage'
function linksToThisPage() {
    if (document.getElementsByTagName("a") !== null) {
        var thispage = document.location.href ? document.location.href : document.location;
        atags = document.getElementsByTagName("a");
        for (var i=0;  i<atags.length;  i++) {
            if(atags[i].href == thispage) {
                atags[i].className = "thispage " + atags[i].className;
            }
        }
    }
}
jQuery(linksToThisPage);


// global variables used by show/hide comments functions - Fixed Left Globals
FLG_comments = null; // array of comments within wiki page content
FLG_buttons = null; // array of Comments hyperlink in sidebar (only expect one)

// involked on page load and after user clicks on Comments hyperlink, shows/hides comment status and links to individual comments
function toggleCommentLinks(display) {
    var nbrComments = document.getElementById('nbrComments');
    var shownImg;
    var hiddenImg;
    if (nbrComments && nbrComments.firstChild) {
        // toggle sidebar Comments shown/hidden triangles
        shownImg = nbrComments.firstChild;
        if (shownImg.tagName !== 'IMG') {
            // some browsers insert text node before images
            shownImg = shownImg.nextSibling;
        }
        hiddenImg = shownImg.nextSibling;
        // toggle visibility of hyperlinks to individual comments
        var ul = document.getElementById('putCommentLinksHere');
        var li = ul.firstChild;
        while(li) {
            li.style.display = display;
            li = li.nextSibling;
        }
    } else {
        var iconbarComments = document.getElementById('iconbarComments');
        if (iconbarComments) {
            shownImg = iconbarComments.firstChild;
            hiddenImg = shownImg.nextSibling;
            iconbarComments.style.display = 'inline';
        }
    }
    if (shownImg && hiddenImg) {
        if (display == 'none') {
            shownImg.style.display = 'none';
            hiddenImg.style.display = '';
        } else {
            shownImg.style.display = '';
            hiddenImg.style.display = 'none';
        }
    }
}

// this is executed when the user clicks the comments button
// Toggle visibility of every tag with class "comment"
// toggleComments overrides function in common.js
function toggleComments() {
    for (i = 0; i < FLG_comments.length; i++){
        var el = FLG_comments[i];
        if ( el.style.display != 'none' ) {
            el.style.display = 'none';
        } else {
            el.style.display = '';
        }
    }
    toggleCommentLinks(FLG_comments[0].style.display);
}

// scan up the DOM looking for a prior element with an id and return the id
function getPriorAnchorId(comment) {
	var node = comment;
	while (!jQuery(node).attr('id')) {
        if (jQuery(node).attr('previousSibling')) {
			node = node.previousSibling;
			while (jQuery(node).attr('lastChild')) {
				node = node.lastChild;
			}
		} else {
			node = node.parentNode;
		}
	}
	return node.id;
}

// show_toggleComments overrides version in common.js
// this is executed on page load via command within common.js
function show_toggleComments() {
    // Show edit bar item for toggling inline comments only if inline comments exist on the page
    FLG_comments = getElementsByClassName('comment', null, document);
    FLG_buttons = getElementsByClassName('toggleCommentsButton', null, document);
    if (FLG_comments.length > 0 && FLG_buttons.length > 0) {
        var count = FLG_comments.length;
		// only 1 button is expected, but an array was returned
        var button = FLG_buttons[0];
        button.style.display = '';
        var nbrComments = document.getElementById('nbrComments');
        var textComment = nbrComments.lastChild;
        textComment.nodeValue = count + ' ' + textComment.nodeValue;
        // build hyperlinks to each comment and insert into sidebar
        ulElement = document.getElementById('putCommentLinksHere');
        for (i = 0; i < FLG_comments.length; i++) {
            var comment = FLG_comments[i];
            anchorId = getPriorAnchorId(comment);
            liElement = document.createElement('li');
            aElement = document.createElement('a');
            aElement.setAttribute('href', '#' + anchorId);
            aElement.innerHTML = comment.innerHTML.slice(0,50);
            liElement.appendChild(aElement);
            ulElement.appendChild(liElement);
        }
    }
    if (FLG_comments.length > 0 ) {
        // comments are displayed per user prefs; make icons and links to comments consistent
        toggleCommentLinks(FLG_comments[0].style.display);
    }
}

// open a wiki page in a new window
function newWikiWindow(t) {
    var href = t.href;
    var winName;
    // IE supports innerText, not textContent
    winName = t.textContent || t.innerText;
    // IE does not allow blanks in window names
    winName = winName.split(' ').join('');
    var newWindow = window.open(href,winName,'width=800,height=600,toolbar=yes,location=yes,directories=no,status=yes,menubar=yes,scrollbars=yes,copyhistory=no,resizable=yes,left=0,top=0');
    newWindow.focus();
    return false;
}

// redirect wiki search form to google
function searchGoogle() {
    var searchInput = document.getElementById('searchinput');
    var wikiUrl = document.getElementById('wikiUrl');
    if (searchInput && searchInput.value && wikiUrl) {
        searchInput = escape(searchInput.value);
        window.location = 'http://www.google.com/search?q=' + searchInput + '&sitesearch=' + wikiUrl.value;
        return false;
    }
}

//  save javascript variables in cookie
function fixedleft_set_cookie (name, value){
    var cookie_string = name + "=" + escape ( value );
    var d = new Date();
    var curr_year = d.getFullYear() + 5;
    var expires = new Date ( curr_year, 1, 1 );
    cookie_string += "; expires=" + expires.toGMTString() + "; path=/";
    document.cookie = cookie_string;
}
function fixedleft_delete_cookie (name){
    var cookie_date = new Date ( );  // current date & time
    cookie_date.setTime ( cookie_date.getTime() - 1 );
    document.cookie = name + "=; expires=" + cookie_date.toGMTString() + "; path=/";
}
function fixedleft_get_cookie (name){
    var results = document.cookie.match ( '(^|;) ?' + name + '=([^;]*)(;|$)' );
    if ( results ) {
        return ( unescape ( results[2] ) );
    } else {
        return null;
    }
}


// ========================================
// code for sidebar transformations starts here

// save sidebar state, both show/hide (inline/none) and width of sidebar
function saveSidebarState(display, width) {
    //display is initially "", make it inline for cookie
    if (!display) {
        display = 'inline';
    }
    fixedleft_set_cookie("FixedLeftThemeSidebar", display + '##' + width);
}

// function to hide sidebar, save left margin value in a cookie, and copy wiki navigation under menu icon
function hideSidebarPart2() {
    var sidebar = document.getElementById('sidebar');
    sidebar.style.display = 'none';
    // change the margin for page content
    var w = document.getElementById('wikipagecontent');
    // should agree with css values
    w.style.marginLeft = '10px';
    // show the show and menu icons
    var altWikiNavMenu = document.getElementById('altWikiNavMenu');
    altWikiNavMenu.style.display = 'inline';
    // move the sidebar navigation menu into the main right page - becomes sideways drop down via css
    var wikiNavMenu = document.getElementById('wikiNavMenu');
    wikiNavMenu = wikiNavMenu.parentNode.removeChild(wikiNavMenu);
    var contentWikiMenu = document.getElementById('contentWikiMenu');
    contentWikiMenu.parentNode.appendChild(wikiNavMenu);
}

// save sidebar state and then hide it; executed when user clicks hidesidebar button
function hideSidebar() {
    // hide the entire sidebar
    var sidebar = document.getElementById('sidebar');
    // subtract 3 for border width - this should agree with CSS
    saveSidebarState('none', sidebar.offsetWidth - 3);
    hideSidebarPart2();
}

// restore sidebar to former width
function showSidebar() {
    var sidebar = document.getElementById('sidebar');
    sidebar.style.display = 'inline';
    // change margin of wiki page content
    var wikipagecontent = document.getElementById('wikipagecontent');
    var cookie = fixedleft_get_cookie("FixedLeftThemeSidebar");
    cookie = cookie.split('##');
    style = cookie[0];
    width = cookie[1];
    wikipagecontent.style.marginLeft = (20 + (width*1)) + 'px';
    sidebar.style.width = width + 'px';
    saveSidebarState(sidebar.style.display, width);
    // hide the show and menu icons
    var altWikiNavMenu = document.getElementById('altWikiNavMenu');
    altWikiNavMenu.style.display = 'none';
    // copy the sidebar navigation menu back to sidebar
    var wikiNavMenu = document.getElementById('wikiNavMenu');
    wikiNavMenu = wikiNavMenu.parentNode.removeChild(wikiNavMenu);
    var sidebarWikiMenu = document.getElementById('sidebarWikiMenu');
    sidebarWikiMenu.appendChild(wikiNavMenu);
}

// change width of sidebar or hide sidebar per previous state saved in cookie
function repeatHide() {
    var cookie = fixedleft_get_cookie("FixedLeftThemeSidebar");
    var wikipagecontent = document.getElementById('wikipagecontent');
    if (cookie && wikipagecontent) {
        // sidebar was changed in one way or another
        cookie = cookie.split('##');
        var style = cookie[0];
        var width = cookie[1];
        if (style == 'inline') {
            // sidebar visible - adjust width and margins
            wikipagecontent.style.marginLeft = ((width*1) + 20) + 'px';
            var sidebar = document.getElementById('sidebar');
            sidebar.style.width = width + 'px';
        } else {
            // sidebar was hidden, hide it again without saving state from page load
            hideSidebarPart2();
        }
    }
}

// add table of contents to sidebar
function tableOfContents() {
    // find all headings within the wiki page
    var pageContent = document.getElementById('page');
    var headerList = jQuery(':header', pageContent);
    var tocPlace;
    var ulElement;
    var liElement;
    var aElement;
    //~ var aaElements;
    var eClass;
    var id;
    if (headerList) {
        // build a structure like this:
        // UL
        //    LI
        //      A
        //    LI
        //      A .....
        tocPlace = document.getElementById('sidebarTOC');
        if (tocPlace) {
            ulElement = document.createElement('ul');
            for (i = 0; i < headerList.length; i++) {
                liElement = document.createElement('li');
                // make class names like sidebarToc_H1, sidebarTOC_H2, ...
                eClass = 'sidebarTOC_' + headerList[i].nodeName;
                liElement.setAttribute('class', eClass);
                aElement = document.createElement('a');
                // use existing ID on an anchor or find a nearby anchor up the DOM
                id = '';
                if (headerList[i].id) {
                    id = headerList[i].id;
                } else {
                    id = getPriorAnchorId(headerList[i]);
                }
                aElement.setAttribute('href', '#' + id);
                // IE supports innerText, not textContent
                aElement.innerHTML = headerList[i].textContent || headerList[i].innerText;
                liElement.appendChild(aElement);
                ulElement.appendChild(liElement);
            }
            // add current heading to sidebar table of contents
            tocPlace.appendChild(ulElement);
        }
    }
}
// execute when DOM complete
jQuery(tableOfContents);


// remember sequence of sidepanels and collapsed sidepanels
function saveSidePanelState() {
    // will hold both a list of panel sequence and a list of collapsed panels
    var idSequenceHide = '';
    // list of panels in current sequence
    var sequence = jQuery("#wikiNavMenu > li");
    // list of collapsed panels
    var hidden = jQuery("#wikiNavMenu > li.hideSidePanel");
    // add panel IDs to list in curent sequence
    for (i = 0; i < sequence.length; i++) {
        idSequenceHide += '#' + sequence[i].id +', ';
    }
    idSequenceHide += '##';
    // add list of collapsed panels
    for (i = 0; i < hidden.length; i++) {
        idSequenceHide += '#' + hidden[i].id +', ';
    }
    // is this the editor -- use different cookies for viewing and editing
    var isEditor = jQuery("div.isEditor").length;
    if (isEditor) {
        fixedleft_set_cookie('FixedLeftThemeEditSequence', idSequenceHide);
    } else {
        fixedleft_set_cookie('FixedLeftThemeViewSequence', idSequenceHide);
    }
    return false;
}

// restore a previously saved sidepanel state
function restoreSidePanelState() {
    var idSequenceHide;
    // is this the editor?
    var isEditor = jQuery("div.isEditor").length;
    if (isEditor) {
        idSequenceHide = fixedleft_get_cookie('FixedLeftThemeEditSequence');
    } else {
        idSequenceHide = fixedleft_get_cookie('FixedLeftThemeViewSequence');
    }
    var cutLi;
    if (idSequenceHide) {
        idSequenceHide = idSequenceHide.split('##');
        var idSequence = idSequenceHide[0];
        // place in prior sequence by cutting and appending
        var wikiNavMenu = document.getElementById('wikiNavMenu');
        // chop off leading "#" and trailing ", "
        idSequence =idSequence.slice(1,-2);
        idList = idSequence.split(', #');
        // restore panel sequence
        for (i = 0; i < idList.length; i++) {
            if (document.getElementById(idList[i])) {
                cutLi = wikiNavMenu.removeChild(document.getElementById(idList[i]));
                wikiNavMenu.appendChild(cutLi);
            }
        }
        // make all panels visible
        jQuery("#wikiNavMenu > li").removeClass("hideSidePanel");
        jQuery("#wikiNavMenu > li").addClass("showSidePanel");
        if (idSequenceHide[1]) {
            // get list of IDs that should be hidden and hide them
            var idHide = idSequenceHide[1];
            jQuery(idHide).removeClass("showSidePanel");
            jQuery(idHide).addClass("hideSidePanel");
        }
    }
    // panels are now in sequence and expanded/collapsed; now show/hide entire sidebar
    repeatHide();
}
jQuery(restoreSidePanelState);

// collapse/expand sidebar panels (li.sidepanel) by alternating show/hide css class
function toggleShowHide(that) {
    var ele = that.parentNode;
    if (jQuery(ele).hasClass("hideSidePanel")) {
        jQuery(ele).removeClass("hideSidePanel");
        jQuery(ele).addClass("showSidePanel");
    } else {
        jQuery(ele).removeClass("showSidePanel");
        jQuery(ele).addClass("hideSidePanel");
    }
}
// assign a click event to show/hide panel contents
function makeSidePanelsCollapsible() {
    jQuery('li.sidepanel div').click(
        function () {
            toggleShowHide(this);
            saveSidePanelState();
        }
    );
}
// execute when DOM is complete
jQuery(makeSidePanelsCollapsible);

// make .sidepanel LI items sortable by dragging them up and down
//jQuery(function() {
//    jQuery("#wikiNavMenu").sortable({
//        update: function(event, ui) {
//            saveSidePanelState();
//        }
//    });
//});

// adjust left margin of wikipagecontent after resizing width of  sidebar
function sidebarWasResized(event, ui) {
    // side effect of resizing sidebar width is fixing height in pixels, set it back to 100%
    var sidebar = document.getElementById('sidebar');
    sidebar.style.height='100%';
    // set left margin of wikipagecontent to width of sidebar + 20 px
    var wikipagecontent = document.getElementById('wikipagecontent');
    wikipagecontent.style.marginLeft = 20 + sidebar.offsetWidth + 'px';
    saveSidebarState(sidebar.style.display, sidebar.offsetWidth - 3);
}

// make sidebar resizeable
jQuery(document).ready(function(){
    jQuery("div#sidebar").resizable({
        handles: 'e',
        minWidth: 100,
        maxWidth: 1000,
        stop: function(event, ui) {
            sidebarWasResized(event, ui);
        }
    });
});
