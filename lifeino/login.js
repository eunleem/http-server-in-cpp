$(function() {
  checkLoggedIn();

  $("#btnLogin").click(function() {
    var code = $("#txtCode").val().replace(/ /g, "");
    $.ajax({
      type: "POST",
      url: "/login",
      data: {
        "code": code
      }

    }).fail(function(){
      alert("failed");

    }).done(function(result) {
      console.log(result);
      if (result["code"] == -1) {
        alert("failed -1");

      } else if (result["code"] == -2) {
        alert("failed -2");

      } else if (result["code"] == -3) {
        alert("failed -3");

      } else if (result["code"] == 0) {
        document.location.href = "/main";
      } else {
        alert("failed else");

      }
    }).always(function() {
    });

  });
});

function checkLoggedIn() {
  $.ajax({
    type: "POST",
    url: "/login"

  }).fail(function(){
    alert("failed");

  }).done(function(result) {
    console.log(result);
    if (result["code"] == 1) {
      jGet("#boxLogin").html("You are aleady Logged In!");

    } else if (result["code"] == -1) {

    } else if (result["code"] == -2) {

    } else if (result["code"] == 0) {
    } else {
      alert("failed else");
    }
  }).always(function() {
  });
}
