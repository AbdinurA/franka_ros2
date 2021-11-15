pipeline {
    agent {
         dockerfile true 
    }
    stages {
        stage('Init') {
            steps {
                script {
                    notifyBitbucket()
                }
            }
        }
        stage('Build') {
            steps {
                sh '. /opt/ros/foxy/setup.sh && colcon build '
              
            }
        }
        stage('Test') {
            steps {
                sh '. /opt/ros/foxy/setup.sh && . install/setup.sh && colcon test && colcon test-result'    
            }
        }
    }
    post {
        always {
            script {
                notifyBitbucket()
            }
        }
    }
}
