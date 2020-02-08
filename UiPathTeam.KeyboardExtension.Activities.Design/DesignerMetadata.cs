using System.Activities.Presentation.Metadata;
using System.ComponentModel;

namespace UiPathTeam.KeyboardExtension.Activities.Design
{
    public class DesignerMetadata : IRegisterMetadata
    {
        public void Register()
        {
            var builder = new AttributeTableBuilder();
            var category = new CategoryAttribute(Resource.ActivityCategory);
            builder.AddCustomAttributes(typeof(Initialize), category);
            builder.AddCustomAttributes(typeof(Uninitialize), category);
            builder.AddCustomAttributes(typeof(Configure), category);
            builder.AddCustomAttributes(typeof(Restore), category);
            builder.AddCustomAttributes(typeof(Apply), category);
            MetadataStore.AddAttributeTable(builder.CreateTable());
        }
    }
}
