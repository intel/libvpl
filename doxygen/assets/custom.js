'use strict';

$(window).on('load', function() {
    $('#side-nav').resizable( "destroy" );
    $('#nav-btn').on('click', function() { $('#side-nav').css('display', 'flex') });
});
