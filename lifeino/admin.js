$(function() {
  getinvitations();
  getusers();

  $("#btnAddInvit").click(function() {
    addinvitation();
  });

});

function getinvitations() {
  $("#tblInvitations > tbody").html("");

  $.ajax({
    type: "POST",
    url: "/admin",
    data: {
      "action": "getinvitations"
    }

  }).fail(function(){
    alert("failed");

  }).done(function(result) {
    console.log(result);
    var rows = result.split("\n");
    console.log(rows);
    for (var i in rows) {
      var data = rows[i].split("::");
      var row = $("<tr>", { "invitid": data[0] });
      for (var j in data) {
        row.append($("<td>", { "text": data[j].toString() }))
      } 
      $("#tblInvitations > tbody").append(row);
    } 
  }).always(function() {
  });

}

function addinvitation() {
  var numTickets = $("#txtNewInvitNumTickets").val();
  var expHours = $("#txtNewInvitExpHours").val();

  $.ajax({
    type: "POST",
    url: "/admin",
    data: {
      "action": "addinvitation",
      "numtickets": numTickets,
      "expirationhours": expHours
    }

  }).fail(function(){
    alert("failed");

  }).done(function(result) {
    getinvitations();
  }).always(function() {
  });

}

function getusers() {
  $("#tblUsers > tbody").html("");

  $.ajax({
    type: "POST",
    url: "/admin",
    data: {
      "action": "getusers"
    }

  }).fail(function(){
    alert("failed");

  }).done(function(result) {
    console.log(result);
    var rows = result.split("\n");
    console.log(rows);
    for (var i in rows) {
      var data = rows[i].split("::");
      var row = $("<tr>", { "userid": data[0] });
      for (var j in data) {
        row.append($("<td>", { "text": data[j].toString() }))
      } 
      $("#tblUsers > tbody").append(row);
    } 
  }).always(function() {
  });

}
