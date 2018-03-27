<?php
    $title = "index";
    include "head.php";
?>
    <link href="assert/css/index.css" rel="stylesheet">
   <div id="father">
       <el-container>
           <el-header>
               <el-button type="text"><span class="glyphicon glyphicon-home"></span></el-button>
           </el-header>
           <el-aside>
               <div class="headlist">
                    <div class="head">
                        <span>Machine</span>
                        <el-tooltip content="refresh" placement="right" effect="light">
                            <el-button type="text" v-on:click="GetMachineList"><span class="glyphicon glyphicon-refresh"></span></el-button>
                        </el-tooltip>
                    </div>
                    <div id="machines" class="list">
                        <button type="button" class="list-group-item" v-for="item in getMachine">
                            {{item.name}}
                        </button>
                    </div>
               </div>
           </el-aside>
           <el-main>
               <div id="child">
                    <el-container>
                        <el-aside>
                        </el-aside>
                        <el-main>

                        </el-main>
                    </el-container>
               </div>
           </el-main>
       </el-container>
   </div>
    <script>
        
        var vm = new Vue({
            el:'#father',
            data:{
                items:[]
            },
            computed:{
                getMachine: function() {
                    return this.items;
                }
            },
            methods:{
                GetMachineList:function () {
                    var vme = this;
                    axios.get('/getMachineList')
                    .then(function(response) {
                        vme.items = response.data;
                    });
                }
            }
        })
        
        vm.GetMachineList();
        
    </script>
<?php
    include "bottom.php";
?>