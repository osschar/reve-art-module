sap.ui.define(['rootui5/eve7/controller/Main.controller',
               'rootui5/eve7/lib/EveManager',
               'rootui5/browser/controller/FileDialog.controller',
               "sap/ui/core/mvc/XMLView",
               'sap/ui/core/Fragment'
], function(MainController, EveManager, FileDialogController, XMLView, Fragment) {
   "use strict";

   return MainController.extend("custom.MyNewMain", {

      onWebsocketClosed : function() {
         var elem = this.byId("centerTitle");
         elem.setHtmlText("<strong style=\"color: red;\">Client Disconnected !</strong>");
      },

      onInit: function() {
         MainController.prototype.onInit.apply(this, arguments);
         this.mgr.handle.setReceiver(this);
         this.mgr.RegisterController(this);
      },

      onEveManagerInit: function() {
         MainController.prototype.onEveManagerInit.apply(this, arguments);
         var world = this.mgr.childs[0].childs;

         // this is a prediction that the fireworks GUI is the last element after scenes
         // could loop all the elements in top level and check for typename

         world.forEach((item) => {
            if (item._typename == "test::Gui") {
               this.fw2gui = item;
               var pthis = this;
               this.mgr.UT_refresh_event_info = function () {
                  pthis.showEventInfo();
               }
               this.showEventInfo();
               return;
            }
         });
      },

      onWebsocketMsg : function(handle, msg, offset)
      {
         this.mgr.onWebsocketMsg(handle, msg, offset);
      },

      showEventInfo : function() {
         let tinfo = this.fw2gui.path + ":" + this.fw2gui.total + "/" + this.fw2gui.count;
         document.title = tinfo;
         let infoLabel = this.getView().byId("infoLabel");
         infoLabel.setText(tinfo);
      },

      nextEvent : function(oEvent) {
          this.mgr.SendMIR("NextEvent()", this.fw2gui.fElementId, "EventManager");
      }
   });
});
