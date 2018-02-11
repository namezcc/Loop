<?php
    $title = "login";
    include "head.php";
?>
	<div id="login">
		<el-container>
			<el-main>
			<div id="app">
				<el-input v-model="user" placeholder="username" class="mgbtom">
					<template slot="prepend" >
						<span class="glyphicon glyphicon-user" aria-hidden="true"></span>
					</template>
				</el-input>
				<el-input v-model="pass" placeholder="password" class="mgbtom">
					<template slot="prepend" >
						<span class="glyphicon glyphicon-lock" aria-hidden="true"></span>
					</template>
				</el-input>
				<el-button type="primary" @click="login" class="mgbtom">登录</el-button>
			</div>
			</el-main>
		</el-container>
	</div>
	<script>
		var vm = new Vue({
			el:'#app',
			data:{
				user:'',
				pass:''
			},
			methods:{
				login:function(){
					var vmt = this;
					axios({
						method: 'post',
						url:'/login',
						data:{
							user:vmt.user,
							pass:vmt.pass
						},
						validateStatus:function(status) {
							return status < 500;
						}
					}).then(function(response) {
						if(response.status===201)
							location.assign(response.headers.location);
						else
							alert(response.status+"  "+response.statusText+" "+response.data);
						console.log(response);
					}).catch(function(error) {
						console.log(error);
					})
				}
			}
		})
	</script>
<?php
    include "bottom.php";
?>