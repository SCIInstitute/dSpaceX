import Drawer from 'material-ui/Drawer';
import React from 'react';
import PropTypes from 'prop-types';
import Workspace from './workspace';
import { withStyles } from 'material-ui/styles';

const drawerWidth = 240;
const styles = theme => ({
  root: {
    flexGrow: 1,
    overflow: 'hidden',
    position: 'relative',
    display: 'flex',
  },
  drawerPaper: {
    position: 'relative',
    width: drawerWidth,
    backgroundColor: '#efe',
    padding: '10px',
  },
  content: {
    flexGrow: 1,
    minWidth: 0,
    backgroundColor: '#eef',
    padding: '10px',
  }
})

/**
 * The top level dSpaceX client component.
 */
class Application extends React.Component {
  constructor(props) {
    super(props);
  }

  componentWillMount() {
    console.log('Initializing Application...');
  }

  componentDidMount() {
    console.log('Application Ready.');
  }

  render() {
    const { classes } = this.props;
    return (
      <div className={classes.root}>
        <Drawer variant="permanent" 
                classes={{ paper: classes.drawerPaper }}>
          Drawer Content here.
        </Drawer>
        <Workspace className={classes.content}>
            Insert Application Here.
        </Workspace>
      </div>
    );
  }
}

// Enforce that Application receives styling.
Application.propTypes = {
  classes: PropTypes.object.isRequired,
};

// Wrap Application in Styling Container.
export default withStyles(styles)(Application);