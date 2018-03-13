import AppBar from 'material-ui/AppBar';
import Button from 'material-ui/Button';
import React from 'react';
import MaterialToolbar from 'material-ui/Toolbar';
import Typography from 'material-ui/Typography';

class Toolbar extends React.Component {
  constructor(props) {
    super(props);
  }

  render() {
    return (
      <AppBar position='absolute' className={this.props.className}>
        <MaterialToolbar>
          <Typography variant="title" color="inherit">
            dSpaceX
          </Typography>
          <div style={{ width: '100%', display: 'flex', flexDirection: 'row-reverse' }}>
            <Button variant="raised" style={{ backgroundColor: 'white' }}
                    onClick={this.props.onConnectClick}
                    disabled={this.props.connectedToServer} >
              Connect
            </Button>            
          </div>
        </MaterialToolbar>
      </AppBar>
    );
  }
}

export default Toolbar;