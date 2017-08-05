$(document).ready(function(){

	// The relative URL of the submit.php script.
	// You will probably have to change	it.
	var submitURL = '/moin_static195/feedback/submit.php';

	// Caching the feedback object:	
	var feedback = $('#feedback');


	$('#feedback .header').click(function(){

		// We are storing the values of the animated
		// properties in a separate object:
				
		var anim	= {		
			mt : -310		// Margin Bottom
		};
		
		var el = $(this).find('.arrow');
		
		if(el.hasClass('down')){
			anim = {
				mt : 0
			};

			// Remove the feedback response
			var response = feedback.find('.response');
			response.hide();

			// Show the text area and restore the submit button
			feedback.find('textarea').show();

			var button = feedback.find('a.submit');
			button.removeClass('working');
			button.show();
		}

		// The first animation moves the form up or down, and the second one 
		// moves the "Feedback heading" so it fits in the minimized version
		
		feedback.find('.section').stop().animate({marginTop: anim.mt}, 
			function() {
				el.toggleClass('down up');
			}
		);
	});
	
	$('#feedback a.submit').live('click',function(){
		var button = $(this);
		var textarea = feedback.find('textarea');
		
		// We use the working class not only for styling the submit button,
		// but also as kind of a "lock" to prevent multiple submissions.
		
		if (button.hasClass('working') || textarea.val().length < 5){
			return false;
		}

		// Locking the form and changing the button style:
		button.addClass('working');
		
		$.ajax({
			url	: submitURL,
			type	: 'post',
			data	: { message : textarea.val()},
			complete	: function(xhr){
				
				var text = xhr.responseText;
				
				// This will help users troubleshoot their form:
				if(xhr.status == 404){
					text = 'Your path to submit.php is incorrect.';
				}

				// Hiding the button and the textarea, after which
				// we are showing the received response from submit.php

				button.fadeOut();

				textarea.fadeOut(function(){
					var span = $('<div>',{
						className	: 'response',
						html		: text
					})
					.hide()
					.appendTo(feedback.find('.section'))
					.show();
				}).val('');
			}
		});
		
		return false;
	});
});
